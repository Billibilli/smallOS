//memory wasteful algorithm

#include "task.h"
#include "page.h"
#include "x86_desc.h"
#include "file_system.h"
#include "lib.h"
#include "i8259.h"
#include "irq0.h"

#define debug_get_new_pcb 0
#define debug_switch_pcb 0
#define debug_remove_pcb 0
#define debug_change_task 0
#define debug_task_num_on 1


extern int screen_x[NUM_TERMINAL];
extern int screen_y[NUM_TERMINAL];

regs_t program_state_b4_task_switch;                 //save run-time state b4 irq0 happened in a particular terminal
regs_t context_regs[NUM_TERMINAL];                   //save run-time state b4 syscall
regs_t parent_of_current_task[NUM_TERMINAL];         //when a execute happens, this is a backup of run-time state b4 execute runs



static pcb_t* pcb_list_head[NUM_TERMINAL] ;          //currently executing program in each terminal
static uint32_t task_bitmap[NUM_OF_TASK_TOTAL]={0};       //bitmap of the task in each terminal
static int32_t inode_init;                           //only for initialization of the pcbs for each terminal

volatile uint32_t cur_terminal = 0;                  //currently running program in scheduleing time slice
volatile uint32_t alt_terminal = 0;                  //the terminal we are looking at
volatile uint32_t system_init_flag = 0;              //system initialization flag.1 means
//volatile uint32_t irq0_flag =0;


//internal function
static void system_terminal_init();
static const char haltimg0 []= "\\(^_^\\)~~~~~~~~~~~~~~~~~~~~~~~~~~~\\(>o<)/~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~(/^_^)/\n";
static const char haltimg1 []= "\\(^_^\\)---------------The operating systam is halted-------------------(/^_^)/\n";
static const char haltimg2 []= "\\(^_^\\)                                                                (/^_^)/\n";
/* Swtich_task
 * Description: It maps the data into video mmemory and increment the current task 
 * 		 Input: -- cur_terminal: The identity of the current terminal, i.e: Terminal 1, Terminal 2, etc. 
 *       Output: Increment the current terminal number
 *		 Return value: N/A
 * 		 Side effect: allocate page for the user lvl, and switch the task. 
 */
void 
switch_task()
{

		uint32_t stack_ptr_temp;
		
		/* Store the regs info before switching into pcb.*/
		pcb_list_head[cur_terminal]->regs=program_state_b4_task_switch;
		
		/* Temp variable for storing the esp*/
		stack_ptr_temp=pcb_list_head[cur_terminal]->regs.esp;
		/* Map the video mem from vitrual to physical */
		user_page_mapper_4K((uint32_t)0xB8000,(uint32_t)0xB8000);
		/* Map the video mem for the displaying terminal which going to switch. */
		user_page_mapper_4K((uint32_t)0xB8000+(alt_terminal+1)*size_of_vmem,(uint32_t)0xB8000+(alt_terminal+1)*size_of_vmem);
		/* Map the video mem for the running terminal which going to switch. */
		user_page_mapper_4K((uint32_t)(uint32_t)0xB8000+(cur_terminal+1)*size_of_vmem,(uint32_t)0xB8000+(cur_terminal+1)*size_of_vmem);
		if(alt_terminal==cur_terminal)
		/* Memory copy from 0xB800 to the video mem of running terminal*/
		memcpy((void*)0xB8000+(cur_terminal+1)*size_of_vmem, (void*)0xB8000, size_of_vmem);
		/* Else, increment the running terminal. */
		cur_terminal++;
		/* If it reaches the max, then switch back to terminal 0*/ 
		if(cur_terminal==NUM_TERMINAL)
		cur_terminal=0;
		
		if(alt_terminal==cur_terminal)
		/* Memory copy from 0xB800 to the video mem of displaying terminal*/
		memcpy((void*)0xB8000+(alt_terminal+1)*size_of_vmem, (void*)0xB8000, size_of_vmem);
		
		
		user_page_mapper_4K((uint32_t)0xB8000,(uint32_t)0xB8000+(cur_terminal+1)*size_of_vmem);
		user_page_mapper_4K((uint32_t)0xB8000+(alt_terminal+1)*size_of_vmem,(uint32_t)0xB8000);
		
		//update_cursor(screen_y[alt_terminal],screen_x[alt_terminal]);
		
		__user_space_alloc((pcb_t*)pcb_list_head[cur_terminal]);
		

#if debug_change_task
		printf("\n Going to p_esp:%x,return_addr:%x,cs:%x,ds:%x,es:%x,ss:%x\n",pcb_list_head[cur_terminal]->regs.p_esp,pcb_list_head[cur_terminal]->regs.return_addr,pcb_list_head[cur_terminal]->regs.cs,pcb_list_head[cur_terminal]->regs.ds,pcb_list_head[cur_terminal]->regs.es,pcb_list_head[cur_terminal]->regs.ss);
#endif
		/* Obtain the esp info back to the pcb, for protection purpose. */
		pcb_list_head[cur_terminal]->regs.esp=stack_ptr_temp;
		/* Call switch function to switch pcb. */
		switch_pcb ((pcb_t*)pcb_list_head[cur_terminal]);
}

/* get_inode_init
 * Description: obtain the inode 
 * 		 Input: -- inode: index node from file
 *       Output: obtain the inode
 *		 Return value: N/A
 * 		 Side effect: N/A
 */
void 
get_inode_init(int32_t inode)
{
		inode_init=inode;
}

/* system_terminal_init
 * Description: initialize each terminals 
 * 		 Input: -- inode: index node from file
 *       Output: allocate pages for each terminal video mem
 *		 Return value: N/A
 * 		 Side effect:  start the system timer and shwo the statusbar for each terminal
 */
static void 
system_terminal_init()
{
		uint32_t i;
		uint32_t k=0;
         for (i=0;i<NUM_TERMINAL;i++) {
		  if (i!=0)
		   memcpy((pcb_t*)pcb_list_head[i], (pcb_t*)pcb_list_head[0], sizeof(pcb_t));  //copy the pcb to each terminal task
		   pcb_list_head[i]->kernel_esp = (uint32_t)((pcb_t*)0x007ffffc-i);            //allocate kernel stack for each terminal task
		   pcb_list_head[i]->pid=i;
		   pcb_list_head[i]->parent=pcb_list_head[i];
		   pcb_list_head[i]->child=pcb_list_head[i];
           k=(i-i%SCALE)/SCALE;
		   task_bitmap[k]|=(1<<(i%SCALE));                                             //map to each task_bitmap 
		   user_space_alloc((uint32_t)(fourmb * i));                                   //write program image to each terminal
           read_data(inode_init,0,(uint8_t *)0x08048000,  sizeofinode(inode_init));    //user physical page
		   user_page_mapper_4K((uint32_t)0xB9000+i*size_of_vmem,(uint32_t)0xB9000+i*size_of_vmem);    //allocate pages for each terminal video mem
	     }

		 user_page_mapper_4K((uint32_t)0xB8000,(uint32_t)0xB8000+(cur_terminal+1)*size_of_vmem);      //intialize to first terminal
		 user_page_mapper_4K((uint32_t)0xB8000+(alt_terminal+1)*size_of_vmem,(uint32_t)0xB8000);
		 __user_space_alloc(pcb_list_head[cur_terminal]);
		 /* Call the statusbar func to display message of current terminal. */
		 statusbar();
		 open_IRQ0();
		 cli();
#if debug_task_num_on	 
        int task_count;
		for(k=0;k<NUM_OF_TASK_TOTAL;k++)
		 for(i = 0;i < SCALE;task_count +=(((task_bitmap[k] >>i) & 1)),++i);
#endif
		 
		 system_init_flag=1;
}

/* changetask
 * Description: initialize each terminals 
 * 		 Input: -- inode: index node from file
 *       Output: allocate pages for each terminal video mem
 *		 Return value: N/A
 * 		 Side effect:  start the system timer and shwo the statusbar for each terminal
 */

void 
changetask()
{

		user_page_mapper_4K((uint32_t)0xB8000,(uint32_t)0xB8000);
		user_page_mapper_4K((uint32_t)0xB8000+(alt_terminal+1)*size_of_vmem,(uint32_t)0xB8000+(alt_terminal+1)*size_of_vmem);
		memcpy((void *)0xB8000,(const void *)(0xB8000 + (alt_terminal+1)*size_of_vmem),(uint32_t)size_of_vmem);
		/* Switch the displaying terminal */
		alt_terminal++;
		/* If it reaches the end, go back to 0*/
		if(alt_terminal==NUM_TERMINAL)
		 alt_terminal=0;                              
		/* Map the video mem from vitrual to physical */
		memcpy((void *)0xB8000,(const void *)(0xB8000 + (alt_terminal+1)*size_of_vmem),(uint32_t)size_of_vmem);
		/* Map the video mem for the displaying terminal which going to switch. */
		user_page_mapper_4K((uint32_t)0xB8000,(uint32_t)0xB8000+(cur_terminal+1)*size_of_vmem);
		/* Memory copy from 0xB800 to the video mem of running terminal*/
		user_page_mapper_4K((uint32_t)0xB8000+(alt_terminal+1)*size_of_vmem,(uint32_t)0xB8000);
		/* Update the cursor when switch to a new terminal */
		__user_space_alloc(pcb_list_head[cur_terminal]);
		update_cursor(screen_y[alt_terminal],screen_x[alt_terminal]);
}


/* __user_space_alloc
 * Description: allocat the user space
 * 		 Input: -- context_reg_stack: context of the current terminal
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: store the info
 */

void
__user_space_alloc(pcb_t* pcb)
{
		user_space_alloc((uint32_t)(fourmb * pcb->pid));
}

/* save_context_for_terminal_switch
 * Description: save the context before switching
 * 		 Input: -- context_reg_stack: context of the current terminal
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: store the info
 */
void
save_context_for_terminal_switch(regs_t* context_reg_stack)
{

		//spare the context info
		if (context_reg_stack->irq_num==0x20) 
		{
		/* Store the context reg info b4 task switch*/
		 memcpy((void*)&program_state_b4_task_switch, (void*)context_reg_stack, sizeof(regs_t));
		 program_state_b4_task_switch.esp = (uint32_t)context_reg_stack;
		}
	
}

/* save_context
 * Description: save the context 
 * 		 Input: -- context_reg_stack: context of the current terminal
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: store the info
 */


void 
save_context(regs_t* context_reg_stack)
{
	     uint32_t i;
		 if(!system_init_flag) {
         for (i=0;i<NUM_TERMINAL;i++)
		  pcb_list_head[i]=(pcb_t*)0x007fdffc-i;
		}
		//user_esp at this time instant is the esp pushed by the processor when context switches
		pcb_list_head[cur_terminal]->user_esp=(uint32_t)context_reg_stack->p_esp;
		//spare the context info
		memcpy((void*)&context_regs[cur_terminal], (void*)context_reg_stack, sizeof(regs_t));
		//set current esp point to the stack head
		context_regs[cur_terminal].esp = (uint32_t)context_reg_stack;
}


void 
spare_info_for_halt_for_this_pcb(void)
{
		memcpy((void*)&parent_of_current_task[cur_terminal],(void*)&context_regs[cur_terminal],sizeof(regs_t)); 
}

/* Obtaining the current pcb*/
pcb_t* 
get_cur_pcb(void)
{
		return (pcb_t*)pcb_list_head[cur_terminal];
}

/* get_new_pcb
 * Description: create the new pcb if it is available. 
 * 		 Input: -- cur_terminal: current terminal numbers
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: construct a new pcb 
 */

pcb_t* 
get_new_pcb(void)
{
		pcb_t* new_pcb;
		uint32_t i, alloc_pid;
		uint32_t k=0;
	
#if debug_task_num_on	 
        int task_count;
		for(k=0;k<NUM_OF_TASK_TOTAL;k++)
		 for(i = 0;i < SCALE;task_count +=(((task_bitmap[k] >>i) & 1)),++i);
#endif

	
		for(k=0;(k<NUM_OF_TASK_TOTAL)&&(task_bitmap[k]==0xFFFFFFFF);k++);
		
		/* Check if there's too many tasks */

		if (k==NUM_OF_TASK_TOTAL) {
		 printf("FAIL npcb001:too many taks\n");
		 return NULL;
		}

		/*In first-fit basis, allocate PID */
		for (i=1,alloc_pid=0;(task_bitmap[k] & i)!=0&&alloc_pid<SCALE;i<<=1,alloc_pid++);

		alloc_pid=alloc_pid+k*SCALE;
		
		//Create new pcb
		new_pcb = (pcb_t*)0x007fdffc -alloc_pid;

		//Update task bitmap
		task_bitmap[k]|=(1<<(alloc_pid%SCALE));
	
		//Clear the new pcb	
		memset(new_pcb,0,500);
		
#if debug_get_new_pcb
	    printf("enter memset \n");
#endif	

		//Set pcb struct
		new_pcb->pid=alloc_pid;
		new_pcb->parent=(pcb_t*)pcb_list_head[cur_terminal];    
		new_pcb->parent->child=(pcb_t*)new_pcb;
		//new_pcb->child=NULL;
		new_pcb->kernel_esp=(uint32_t)((pcb_t*)0x7ffffc - alloc_pid);
		new_pcb->user_esp=(uint32_t)0x83ffffc;
		memcpy((void*)&(new_pcb->regs),(void*)&context_regs[cur_terminal], sizeof(regs_t)); 
		new_pcb->regs.cs = USER_CS;
		new_pcb->regs.ss = USER_DS;
		new_pcb->regs.ds = USER_DS;
		new_pcb->regs.es = USER_DS;
		new_pcb->regs.fs = USER_DS;
		new_pcb->regs.gs = USER_DS;
		new_pcb->regs.ebp=(uint32_t)new_pcb->user_esp;
		new_pcb->regs.p_esp=(uint32_t)new_pcb->user_esp;
		new_pcb->regs.eflags=0x200|new_pcb->regs.eflags;
		pcb_list_head[cur_terminal] = new_pcb;

#if debug_task_num_on	 
		for(k=0;k<NUM_OF_TASK_TOTAL;k++)
		 for(i = 0;i < SCALE;task_count +=(((task_bitmap[k] >>i) & 1)),++i);
#endif		
		
		return new_pcb;
}



/* switch_pcb
 * Description: create the new pcb if it is available. 
 * 		 Input: -- cur_terminal: current terminal numbers
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: set the ss as kernel and copy the regs onto kernel stack 
 */
void 
switch_pcb (pcb_t* pcb)
{

#if debug_task_num_on	 
        int task_count,i,k;
		for(k=0;k<NUM_OF_TASK_TOTAL;k++)
		 for(i = 0;i < SCALE;task_count +=(((task_bitmap[k] >>i) & 1)),++i);
#endif

		//if it's the system initializing execute
		if(!system_init_flag)
         system_terminal_init();
		/* Set the tss according to the appendix */
		tss.ss0 = KERNEL_DS;
		tss.esp0 = (uint32_t)(pcb->kernel_esp);

#if debug_switch_pcb
        printf("\npid:%d,parent:%x,child:%x,command_word:%s,arg:%s,ebp:%x,ss:%x,cs:%x,p_esp:%x,return_addr:%x,eflags:%x,list_head:%x,kernel_esp=%x,user_esp:%x,eax:%x\n"
		,pcb->pid,(uint32_t)pcb->parent,(uint32_t)pcb->child,pcb->command_word,pcb->args,pcb->regs.ebp,pcb->regs.ss,pcb->regs.cs,pcb->regs.p_esp,pcb->regs.return_addr
		,pcb->regs.eflags,(uint32_t) pcb_list_head[cur_terminal],pcb->kernel_esp,pcb->user_esp,pcb->regs.eax);
#endif	

		/* Copy the regs info onto kernel stack. */
		memcpy((void*)(pcb->regs.esp),(void*)&pcb->regs, sizeof(regs_t)); 

}

/* remove_pcb
 * Description: remove the pcb 
 * 		 Input: -- cur_terminal: current terminal numbers
 *				-- status: the current status
 *       Output: N/A
 *		 Return value: -- parent->regs.eax : the return addr of the parent.  
 * 		 Side effect: remove from the taks bit map and pcb head points to the parent
 */

int32_t 
remove_pcb(uint8_t status)
{
		int32_t i;
		int32_t task_count=0;
		uint32_t k;
		pcb_t* parent;

		//Get the number of task now
		
		for(k=0;k<NUM_OF_TASK_TOTAL;k++)
		 for(i = 0;i < SCALE;task_count +=(((task_bitmap[k] >>i) & 1)),++i);
		

		if (task_count==1) {
		clear();
		printf("%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s",
		               haltimg0,haltimg2,haltimg2,haltimg2,haltimg2,haltimg2,haltimg2,haltimg2,haltimg2,haltimg2,
					   haltimg1,haltimg2,haltimg2,haltimg2,haltimg2,haltimg2,haltimg2,haltimg2,haltimg2,haltimg0);
		cli();
		while(1);
		}

#if debug_remove_pcb
		//printf("task_count before exit:%d,bit_map:%d\n",task_count,task_bitmap[k]);
#endif
		else if (task_count==0) {
		 printf("FAIL rpcb001:No task at all. You must execute a shell in kernel.c!!\n");
		 return -1;}
		 
		else {	 
		//Remove current process from the bit-map 
		for(k=0; (k<NUM_OF_TASK_TOTAL)&&(task_bitmap[k]==0xFFFFFFFF);k++);
		
		 task_bitmap[k]&=(~(1<<((pcb_list_head[cur_terminal]->pid)%SCALE)));
		
#if debug_remove_pcb
		printf("task_count after exit:%d,bit_map:%d\n",task_count,task_bitmap[cur_terminal]);
#endif
		//Killing no task will raise error
		if (pcb_list_head[cur_terminal]==NULL) {
		 printf("FAIL rpcb002:Attemp to remove a non existed task!\n");
		 return -1;
		}

		//set the pcb list head to parent;
		parent = (pcb_t*)pcb_list_head[cur_terminal]->parent;
		// and copy the related info.
		memcpy((void*)&parent->regs,(void*)&parent_of_current_task[cur_terminal],sizeof(regs_t)); 
		
		//remap page for the parent
		__user_space_alloc(parent);
		
#if debug_remove_pcb
		printf("Trying to remove task from list..");
		printf("\n:::current task info:::\n list_head:%x,tss.esp0:%x\n",(uint32_t)pcb_list_head[cur_terminal],(uint32_t)tss.esp0);
		printf(":::parent pcb general info:::\n pid:%d,command_word:%s,arg:%s,kernel_esp=%x,user_esp:%x,parent:%x,child:%x\n"
		,parent->pid,parent->command_word,parent->args,parent->kernel_esp,parent->user_esp,(uint32_t)parent->parent,(uint32_t)parent->child);
		printf(":::parent pcb reg info::: \nebx:%x,ecx:%x,edx:%x,esi:%x,edi:%x,ebp:%x,esp:%x,eax:%x,ds:%x,es:%x,fs:%x,gs:%x,irq_num:%x,error_code:%x,return_addr:%x,cs:%x,elags:%x,p_esp:%x,ss:%x\n",
	     parent->regs.ebx,parent->regs.ecx,parent->regs.edx,parent->regs.esi,parent->regs.edi,parent->regs.ebp,parent->regs.esp,
	     parent->regs.eax,parent->regs.ds,parent->regs.es,parent->regs.fs,parent->regs.gs,parent->regs.irq_num,parent->regs.error_code,
	     parent->regs.return_addr,parent->regs.cs,parent->regs.eflags,parent->regs.p_esp,parent->regs.ss);		
#endif

		//A fake halter, since there is an unknown bug when the system going back to kernel
		 tss.ss0 = KERNEL_DS;
		 tss.esp0 = (uint32_t)(parent->kernel_esp);
#if debug_remove_pcb		
       printf(":::Tried to return to stack:::\nebx:%x,ecx:%x,edx:%x,esi:%x,edi:%x,ebp:%x,esp:%x,eax:%x,ds:%x,es:%x,fs:%x,gs:%x,irq_num:%x,error_code:%x,return_addr:%x,cs:%x,elags:%x,p_esp:%x,ss:%x\n",
	    parent->regs.ebx,parent->regs.ecx,parent->regs.edx,parent->regs.esi,parent->regs.edi,parent->regs.ebp,parent->regs.esp,
	    parent->regs.eax,parent->regs.ds,parent->regs.es,parent->regs.fs,parent->regs.gs,parent->regs.irq_num,parent->regs.error_code,
	    parent->regs.return_addr,parent->regs.cs,parent->regs.eflags,parent->regs.p_esp,parent->regs.ss);
#endif


		//Kill children;Update linklist
		parent->child = NULL;
		parent->regs.eax = (uint32_t)status&0x0ff;
		pcb_list_head[cur_terminal] = parent;

		pcb_list_head[cur_terminal]->regs.esp=context_regs[cur_terminal].esp;
        switch_pcb ((pcb_t*)pcb_list_head[cur_terminal]);
		 return parent->regs.eax;
		}
}
/* get_vid_start
 * Description: Obtain the staring video mem. 
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: video mem
 * 		 Side effect: N/A 
 */
uint8_t* get_vid_start()
{
if(cur_terminal==alt_terminal)
/* If the displaying terminal is the running terminal,then update the video mem*/
return (uint8_t*)0xB8000+ (alt_terminal + 1)*size_of_vmem;
else 
return (uint8_t*)0xB8000;
}








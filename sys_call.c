#include "sys_call.h"
#include "file_op_table.h"
#include "rtc.h"
#include "task.h"
#include "file_system.h"
#include "page.h"
#define debug_exec 0
#define debug_open 0
#define debug_read 0
#define debug_write 0
#define failure_msg_on 0

////////////////////////////////////////////////sys_call 0/////////////////////////////////////////////////
/* do_test
 * Description: N/A 
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: return 0
 * 		 Side effect: N/A 
 */
int32_t 
do_test()
{
	return 0;
}


////////////////////////////////////////////////sys_call 1/////////////////////////////////////////////////
/* do_halt
 * Description: halt the process. 
 * 		 Input: --status
 *       Output: N/A
 *		 Return value: remove_pcb function call
 * 		 Side effect: N/A 
 */
int32_t 
do_halt (uint8_t status){
	int32_t i;

	//close all files in current process
	for(i=2;i<=7;i++)
	 do_close(i);
	//remove pcb

	return remove_pcb(status);
}


////////////////////////////////////////////////sys_call 2/////////////////////////////////////////////////
/* do_execute
 * Description: execute other function call by dealing with the file name and rest of the command
 * 		 Input: --command: pointer of keyboard input from terminal
 *       Output: N/A
 *		 Return value: return 0 but should not happen. 
 * 		 Side effect: get new pcb, set the correct of regs for kenel stack and perform pcb switch func call
 *					  and write the program to the memory.
 */
int32_t 
do_execute (const uint8_t* command){
	//notice:interrupt mask??
	uint32_t fd, cmd_index,excutable;
	int32_t inode;
	uint8_t command_word[COMM_WORD_LENGTH]={0};
	pcb_t* pcb;

	//-------------------------------Before create a process, check if the process is valid----------------
	//Get the length of the command
	for( cmd_index = 0; 
         command[cmd_index] != ' '  && command[cmd_index] != (uint8_t)"\0" && command[cmd_index] != 0;
         cmd_index++);
	/* If the cmd length is 0, return -1 */	 	 
	if(cmd_index == 0)
	 return -1;
	 /* Copy the string of command into command_word*/ 
	strncpy((int8_t*)command_word, (const int8_t*)command, cmd_index);
	/* Obtain the inode from the function call*/
	inode = get_inode_by_name(command_word);
#if debug_exec
	printf("inode :%x:",inode);
#endif
	/* Check if the inode is valid */
	if(inode ==-1) {
#if failure_msg_on
	 printf("FAIL exec002:do_execute inode returned -1\n",cmd_index);
#endif
	 return -1;}
	 /* If the read file system func does not operate properly, return -1*/
	if(read_data(inode,0,(uint8_t *) &excutable, 0x04) == -1 || excutable != 0x464c457f )
	{
#if failure_msg_on
	 printf("FAIL exec003:program loaded is not executable or read_data failed\n");
#endif
	 return -1;
	}
	//------------------------------Spare info for halt-----------------------------------------------------

	spare_info_for_halt_for_this_pcb();
	
	//------------------------------Attempt to get process(If this fail no process will be created----------
	pcb=get_new_pcb();
	/* Check if pcb is valid*/	
	if (pcb==NULL) {
#if failure_msg_on
	 printf("FAIL exec001:do_execute pcb is NULL\n",cmd_index);
#endif
	 return -1;}

#if debug_exec
	printf("cmd_index %d\n",cmd_index);
#endif
    
	//Copy the command to the name buffer,probably bug
	strncpy((int8_t*)pcb->command_word, (const int8_t*)command_word, cmd_index);

	//Build pcb by storing pcb->arg,probably bug
	if(strlen((int8_t*)command) > (cmd_index + 1))
	strncpy((int8_t *)pcb->args, (const int8_t*)command + cmd_index + 1, strlen((int8_t*)command)-cmd_index-1);

#if debug_exec
	printf("enter do_execute done with read by name\n %d cmd:  pcb->command_word  \%s\ cmb %s\n", inode,pcb->command_word,command);
#endif

	//Check if file is excutable 
    for( fd = 0; fd<8; fd++)
	{
    pcb->file_array[fd].flags = 0;
	}
	
	//Stdin,0 for terminal, set the appropriate parameter
	pcb->file_array[0].file_operations_pointer = &terminal_type_op;
	pcb->file_array[0].inode_pointer = 0;
	pcb->file_array[0].file_position =0;
	pcb->file_array[0].flags = 1;

	//Stdout, 1 for keyboard, set the appropriate parameter
	pcb->file_array[1].file_operations_pointer = &terminal_type_op;
	pcb->file_array[1].inode_pointer = 0;
	pcb->file_array[1].file_position =0;
	pcb->file_array[1].flags = 1;

	//Done kernel stuff. Allocate page for user_used space 
	__user_space_alloc(pcb);
	
#if debug_exec
	printf("enter do_execute done copy program to memory\n");
#endif

	//Write the program to the memory
	read_data(inode,0,(uint8_t *)0x08048000,  sizeofinode(inode));
#if debug_exec
	printf("enter do_execute done copy program to memory\n");
#endif
	
#if debug_exec
	printf("command_word : %s\n" , pcb->command_word);
#endif

#if debug_exec
	printf("first instruction of this program: %x\n" , *(uint32_t *)(pcb->regs.return_addr));
#endif

	//Get the new eip which is 24-27 bytes as the program entry point
	pcb->regs.return_addr=(uint32_t)*(uint32_t *)0x08048018; 
	
	//initialization process
	get_inode_init(inode);
	
	//Switch process, enter the program  
	switch_pcb(pcb);
	
	//This return should never happen
return 0;
}




////////////////////////////////////////////////sys_call 3/////////////////////////////////////////////////
/* do_read
 * Description: read the file to the end of the file, or return the line has been terminated
 * 		 Input: --fd: index of file array
 *				--buf: pointer to where to read 
 *				--nbutes: length need to read. 
 *       Output: N/A
 *		 Return value: return the number of bytes read or -1 if failed 
 * 		 Side effect: N/A
 */
int32_t 
do_read (int32_t fd, void* buf, int32_t nbytes){
#if debug_read
	//printf("entering read: %d",fd);
#endif
	/* Obtain the current pcb. */
	pcb_t* pcb = get_cur_pcb();
	/* Check the condition of the file.*/
	if(fd >= 8 || buf == NULL || pcb->file_array[fd].flags == 0 || pcb->file_array[fd].file_operations_pointer == NULL||fd==1)
#if failure_msg_on
	printf("FAIL read001:failed\n" );
#endif
	return -1;
#if debug_read
	int32_t K;
	K=((file_type_op_action_t *) (pcb->file_array[fd].file_operations_pointer))-> read((filedescriptor_t*)&pcb->file_array[fd],buf,(uint32_t)nbytes);
	printf("read_buf %s\n",buf);
#endif
	/* return the number of bytes read. */
	return ((file_type_op_action_t *) (pcb->file_array[fd].file_operations_pointer))-> read((filedescriptor_t*)&pcb->file_array[fd],buf,(uint32_t)nbytes);
}





////////////////////////////////////////////////sys_call 4/////////////////////////////////////////////////
/* do_write
 * Description: write the data to the terminal or to a device
 * 		 Input: --fd: index of file array
 *				--buf: pointer to where to write 
 *				--nbutes: length need to write. 
 *       Output: N/A
 *		 Return value: return the number of bytes write or -1 if failed 
 * 		 Side effect: N/A
 */
int32_t 
do_write (int32_t fd, const void* buf, int32_t nbytes){
#if debug_write
	printf("entering write %d ",fd);
#endif
	/* Obtain the current pcb. */
	pcb_t* pcb = get_cur_pcb();
	/* Check the condition of the file.*/
	if(fd >= 8 || buf == NULL || pcb->file_array[fd].flags == 0 || pcb->file_array[fd].file_operations_pointer == NULL||fd==0)
#if failure_msg_on
	printf("FAIL writ001:failed\n" );
#endif
	return -1;
	/* return the number of bytes write. */
	return ((file_type_op_action_t *) (pcb->file_array[fd].file_operations_pointer))-> write((filedescriptor_t*)&pcb->file_array[fd],buf,(uint32_t)nbytes);
}





////////////////////////////////////////////////sys_call 5/////////////////////////////////////////////////
/* do_open
 * Description: provides the access of the file. 
 * 		 Input: --filename: the file that needs access to
 *       Output: N/A
 *		 Return value: return the file descriptor index or -1 if failed 
 * 		 Side effect: N/A
 */
int32_t 
do_open (const uint8_t* filename){
	int32_t fd;
	dentry_t dentry;

#if debug_open
	printf("filename : %s\n" , filename);
#endif
	/* Obtain the dentry by the following func call*/
	read_dentry_by_name(filename,&dentry);
#if debug_open
	printf("file read success : %s\n" , filename);
#endif
	uint32_t inode = get_inode_by_name((uint8_t*)filename);
	/* Obtain the current pcb*/
	pcb_t* pcb = get_cur_pcb();

	/* Search for the file which is accessible */
	for( fd = 2; fd<8; fd++)
	{
	if(pcb->file_array[fd].flags == 0)
	break;
	}
	
	if(fd == 8)

	{
#if failure_msg_on
	printf("FAIL open001:too many opened" );
#endif
	return -1; 

	}
	/* Set up the file according to dentry type. */
	if(dentry.type == 0)
	{
		pcb->file_array[fd].file_operations_pointer =  &RTC_type_op;//file_type_op table address
		pcb->file_array[fd].inode_pointer = 0;
		pcb->file_array[fd].file_position = 0;
		pcb->file_array[fd].flags = 1;
	}
	if(dentry.type == 1)
	{
		pcb->file_array[fd].file_operations_pointer =  &directry_type_op;//file_type_op table address
		pcb->file_array[fd].inode_pointer = 0;
		pcb->file_array[fd].file_position = 0;
		pcb->file_array[fd].flags = 1;
	}
	if(dentry.type == 2)
	{
		pcb->file_array[fd].file_operations_pointer =  &file_type_op;//file_type_op table address
		/* For a "file" type, set up the inode obtained from above. */
		pcb->file_array[fd].inode_pointer = inode;
		pcb->file_array[fd].file_position = 0;
		pcb->file_array[fd].flags = 1;
	}
#if debug_open
		printf("fd : %d\n" , fd);
#endif
		/* Return fd which its file has been set to accesible*/
		return fd;
	
}




////////////////////////////////////////////////sys_call 6/////////////////////////////////////////////////
/* do_close
 * Description: set the specific file descriptor's flag and be ready to reopen
 * 		 Input: --fd: the specific file descriptor
 *       Output: N/A
 *		 Return value: return 0 if succeed or -1 if failed 
 * 		 Side effect: N/A
 */
int32_t 
do_close (int32_t fd)
{
	/* Obtain the current pcb */
	pcb_t* pcb = get_cur_pcb();
	if(fd >= 8 || fd <2) {
#if failure_msg_on
	printf("do_close invalid fd\n" );
#endif
	 return -1;}
	/*Set the position to 0 as well as the flag*/ 
	pcb->file_array[fd].file_position = 0;
	pcb->file_array[fd].flags = 0;
	return 0;
}



////////////////////////////////////////////////sys_call 7/////////////////////////////////////////////////
/* do_getargs
 * Description: get the args from the command line except the first word and store them into the buffer
 * 		 Input: --buf: the buffer which store the data from command line
 *				--nbytes: the length of the bytes that needs to store
 *       Output: N/A
 *		 Return value: return 0 if succeed or -1 if failed 
 * 		 Side effect: N/A
 */
int32_t 
do_getargs (uint8_t* buf, int32_t nbytes)
{
	/* Obtain the current pcb */
	pcb_t* pcb = get_cur_pcb();
	if (strlen((const int8_t*)(pcb->args)) > nbytes || nbytes < 1)   {
#if failure_msg_on
	 printf("FAIL getag001:try to get too more arg or too few bytes\n");
#endif
	 return -1;}
	 /* Copy the arguments into the buffer. */
     strncpy((int8_t*)buf, (const int8_t*)(pcb->args), nbytes);
	return 0;
}




////////////////////////////////////////////////sys_call 8/////////////////////////////////////////////////
/* do_vidmap
 * Description: maps the text-mode video memory into user space
 * 		 Input: --screen_start: starting addr of the screen
 *       Output: the starting memory has been maps to the correct video memory 
 *		 Return value: return 0 if succeed or -1 if failed 
 * 		 Side effect: N/A
 */
int32_t 
do_vidmap (uint8_t** screen_start)
{
	/* Check if the addr is within the program image*/
	if(screen_start < (uint8_t**) 0x08000000 || screen_start > (uint8_t**) 0x08400000)
#if failure_msg_on
	printf("FAIL dovid001:fail\n");
#endif
	return -1;
	/* Maps the video memory*/
	*screen_start = (uint8_t*) get_vid_start(); 
	return 0;
}



////////////////////////////////////////////////sys_call 9/////////////////////////////////////////////////
/* do_set_handler
 * Description: N/A
 * 		 Input: --signum: N/A
 *				--handler_address: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: N/A
 */
int32_t 
do_set_handler (int32_t signum, void* handler_address)
{
return -1;
}



////////////////////////////////////////////////sys_call 10/////////////////////////////////////////////////
/* do_sigreturn
 * Description: N/A
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: N/A
 */
int32_t 
do_sigreturn (void)
{
return -1;
}








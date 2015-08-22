#ifndef _TASK_H
#define _TASK_H

#include "types.h"
#include "file_desc.h"
#define COMM_WORD_LENGTH 64
#define ARG_LENGTH 64
#define size_of_vmem 4096

#define NUM_TERMINAL 3
#define NUM_OF_TASK 68
#define SCALE 32 
#define NUM_OF_TASK_TOTAL (int)(NUM_OF_TASK/SCALE + NUM_TERMINAL/SCALE+1)

//Register stack
typedef struct{
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
	uint32_t ebp;
	uint32_t esp;
    uint32_t eax;
    uint32_t ds;
    uint32_t es;
    uint32_t fs;
    uint32_t gs;
    uint32_t irq_num;
    uint32_t error_code;
    uint32_t return_addr;
    uint32_t cs;
    uint32_t eflags;
    uint32_t p_esp;
    uint32_t ss;
}regs_t;

//Process Control Block Structure
union pcb__t{
struct{
   uint32_t pid;
   filedescriptor_t file_array[8];
   uint8_t command_word[COMM_WORD_LENGTH];
   uint8_t args[64];
   uint32_t kernel_esp;
   uint32_t user_esp;
   union pcb__t* parent;
   union pcb__t* child;
   union pcb__t* nxt_task;
   union pcb__t* prv_task;
   regs_t regs;                       //run-time state of the program
};
uint32_t ker_stack[2048];
};
typedef union pcb__t pcb_t;

extern void changetask();
extern pcb_t* get_new_pcb(void);
extern pcb_t* get_cur_pcb(void);
extern void switch_pcb (pcb_t* pcb); 
extern int32_t remove_pcb(uint8_t status);
extern void save_context(regs_t* context_reg_stack);
extern void spare_info_for_halt_for_this_pcb(void);
extern void __user_space_alloc(pcb_t* pcb);
extern void get_inode_init(int32_t inode);
extern volatile uint32_t cur_terminal;
extern volatile uint32_t alt_terminal;
extern void save_context_for_terminal_switch(regs_t* context_reg_stack);
extern void switch_task();
extern uint8_t* get_vid_start();
extern volatile uint32_t sys_init_flag;
#endif









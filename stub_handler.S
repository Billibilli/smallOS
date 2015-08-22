.text
.global irq0
.global irq1
.global irq8
.global system_call
#--------------------------------------------------------------------------
#=====stack=====#
#  	ebx			#
#	ecx			#
#	edx			#
#	esi			#
#	edi			#
#	ebp			#
#	esp			#
#	eax			#
#	ds			#
#	es			#
#	fs			#
#	gs			#
#	int number	#
#	err code	#
#	ret addr (p)#
#	cs		 (p)#
#	eflags	 (p)#
#	p_esp	 (p)#
#	ss		 (p)#
#===============#




# -----------------------------------------------------irq-------------------------------------------------------------
# void irq0:(void);
# Description:  do irq0 function call
#  Interface :  Register-based arguments (not C-style)
#     Inputs :  push 0, push 0x20
#    Outputs :  do irq0 function call
# Registers: 
.align 4
irq0:
pushl $0                          #dummy error code
pushl $0x20                       #irq_num
jmp common_handler
# void irq1:(void);
# Description:  do irq1 function call
#  Interface :  Register-based arguments (not C-style)
#     Inputs :  push 0, push 0x21
#    Outputs :  do irq1 function call
# Registers: 
.align 4
irq1:
pushl $0                          #dummy error code
pushl $0x21                       #irq_num
jmp common_handler
# void irq8:(void);
# Description:  do irq8 function call
#  Interface :  Register-based arguments (not C-style)
#     Inputs :  push 0, push 0x28
#    Outputs :  do irq8 function call
# Registers: 
.align 4
irq8:
pushl $0                          #dummy error code
pushl $0x28                       #irq_num
jmp common_handler

# -----Context Save (stack bottom is the processor saved context)

.align 4
common_handler:
pushl %gs
pushl %fs
pushl %es
pushl %ds
pushl %eax
pushl %esp
pushl %ebp
pushl %edi
pushl %esi
pushl %edx
pushl %ecx
pushl %ebx

# -----Spare the context info
pushfl 
pushl %edx                             #spare the regs to be used by the system call
pushl %ecx
pushl %ebx
pushl %eax
leal 20(%esp),%eax
pushl %eax                             #push argument, which is a ptr to the context regs
call save_context_for_terminal_switch  #save context
addl $4,%esp					       #pop argument
popl %eax
popl %ebx
popl %ecx
popl %edx
popfl

# -----Data segment switch to kernel mode
pushl %eax						   #cs set by idt entry,ss and esp set by tss
movw $0x18,%ax                     
movw %ax,%ds
movw %ax,%es
movw %ax,%gs
movw %ax,%fs
popl %eax

# -----Do irq call functions
movl 48(%esp),%eax
call *irq_except_table(,%eax,4)      #look up jump table to find desired function


# -----Context Restore
popl %ebx						     #pop regs
popl %ecx
popl %edx
popl %esi
popl %edi
popl %ebp
addl $4,%esp
addl $4,%esp   
popl %ds
popl %es
popl %fs
popl %gs
addl $8,%esp                         #pop arguments   
iret



.align 4
irq_except_table:
#---exceptions---
.long 0
.long 0
.long 0
.long 0
.long 0
.long 0
.long 0
.long 0
.long 0
.long 0
.long 0
.long 0
.long 0
.long 0
.long 0
.long 0
.rept 16
.long 0
.endr	
#---irqs---
.long do_irq0
.long do_irq1
.long 0
.long 0
.long 0
.long 0
.long 0
.long 0
.long do_irq8
.long 0
.long 0
.long 0
.long 0
.long 0
.long 0
.long 0




# -----------------------------------------------------system call-------------------------------------------------------------
# void system_call:(void);
# Description:  perform system call otherwise returns
#  Interface :  Register-based arguments (not C-style)
#     Inputs :  0x80, init sys call.
#    Outputs :  jump to the desired function call according 
#	     		 to jump table, perform kernel/user stack switch
# Registers: gs,fs,es,ds,eax,esp,ebp,edi,esi,edx,ecx,ebx
.align 4
system_call:
# -----Context Save (stack bottom is the processor saved context)

cli
cmpl $10,%eax                     #system service number validity check
jae badsys
pushl $0                          #dummy error code
pushl $0x80                       
pushl %gs
pushl %fs
pushl %es
pushl %ds
pushl %eax
pushl %esp
pushl %ebp
pushl %edi
pushl %esi
pushl %edx
pushl %ecx
pushl %ebx


# -----Spare the context info
pushfl 
pushl %edx                         #spare the regs to be used by the system call
pushl %ecx
pushl %ebx
pushl %eax
leal 20(%esp),%eax
pushl %eax                         #push argument, which is a ptr to the context regs
call save_context                  #save context
addl $4,%esp					   #pop argument
popl %eax
popl %ebx
popl %ecx
popl %edx
popfl

# -----Data segment switch to kernel mode
pushl %eax						   #cs set by idt entry,ss and esp set by tss
movw $0x18,%ax                     #store 0x18 onto the user stack 
movw %ax,%ds
movw %ax,%es
movw %ax,%gs
movw %ax,%fs
popl %eax

# -----Do system call functions
pushl %edx                         #push arguments
pushl %ecx
pushl %ebx
call *sys_call_table(,%eax,4)      #look up jump table to find desired function
popl %ebx
popl %ecx
popl %edx


# -----Context Restore
popl %ebx						   #pop regs
popl %ecx
popl %edx
popl %esi
popl %edi
popl %ebp
addl $4,%esp
addl $4,%esp   						#update esp
popl %ds
popl %es
popl %fs
popl %gs
addl $8,%esp                       #pop arguments   
iret

.align 4
badsys:								#bad system call
movl $-1,%eax
iret

.align 4		
sys_call_table:						#jump table
.long do_test
.long do_halt
.long do_execute
.long do_read
.long do_write
.long do_open
.long do_close
.long do_getargs
.long do_vidmap
.long do_set_handler
.long do_sigreturn












.text
.global paging
.global user_space_alloc
.global flush_tlb
.global user_page_mapper_4K
.global user_page_mapper_4M
#|-----------------------------------Glabal Parameters-----------------------------------------|
Kernel:
.long 0x00400000                    #virtual addr of kernel
Program_img:                         
.long 0x08000000                    #virtual addr of program img
Program_phys:						#physical addr of program img
.long 0x00800000
Video_mem:
.long 0x000B8000                    #virtual addr of video mem
#|--------------------------------Allocate local static physical memory -----------------------|

.align 4096
_PageDir:
PageDir:
.rept 1024
.long 0
.endr	

.align 4096
_PageTbl:
PageTbl:
.rept 1024
.long 0
.endr	

#|--------------------------------End of allocation of memory --- -----------------------------|
# void paging:(void);
# Description:  allocate page for the system
#  Interface :  Register-based arguments (not C-style)
#     Inputs :  Program_img
#				Program_phys
#				Video_mem
#				PageDir
#				PageTbl
#    Outputs :  dynamically allocated page in virtual memory
# Registers: gs,fs,es,ds,eax,esp,ebp,edi,esi,edx,ecx,ebx
paging:
pushl %ebp                           #construct a stack
movl %esp, %ebp
pushl %edi
pushl %ebx
pushl %eax

init_ker:
movl Kernel,%ebx                     #fetch data
movl %ebx,%eax                       #prepare the entry
shrl $22,%ebx                        #get index of page dir(higher bits)
andl $0xFFC00000,%eax                #get to-be-written physaddr bits 31-22,16-13
orl  $0x085, %eax                    #set U/S(bit 2),R/W(bit 1),P(bit 0) and PSE(bit 7) 
leal PageDir,%edi                    #load page dir
movl %eax,(%edi,%ebx,4)              #build the entry



#|-------------------------------Initialize video mem------------------------------------------|
init_vmem:
movl Video_mem,%ebx                  #fetch data
movl %ebx,%eax                       #prepare the entry
shrl $22,%ebx                        #get index of page dir(higher bits)
xorl %eax, %eax                      #entry for dir
orl $0x07, %eax                      #set U/S(bit 2),R/W(bit 1),P(bit 0)
leal PageTbl,%edi                    #get the page table 
orl %edi,%eax                        #load the table 
leal PageDir,%edi                    #load page dir
movl %eax,(%edi,%ebx,4)              #build the entry

movl Video_mem,%ebx                  #fetch data
movl %ebx,%eax                       #prepare the entry
shrl $12,%ebx                        #get index of page dir(higher bits)
andl $0x000003FF,%ebx                #get the lower bits as indes of page table
andl $0x003FF000,%eax                #the page physical addr's higher bits
orl $0x07,%eax                       #set U/S(bit 2),R/W(bit 1),P(bit 0)
leal PageTbl,%edi                    #load page dir
movl %eax,(%edi,%ebx,4)              #build the entry

# -------------------------------------Load Regs------------------------------------------------
init_cr3:
leal PageDir ,%eax
andl $0xfffff000,%eax
movl %cr3, %ebx
andl $0x00000fff,%ebx
orl  %eax, %ebx
movl %ebx,%cr3

init_cr4:
movl %cr4, %eax
orl  $0x00000010,%eax
movl %eax, %cr4

init_cr0:
movl %cr0, %eax	
orl  $0x80000001,%eax
movl %eax, %cr0
	
# -------------------------------------Load Regs------------------------------------------------
popl %eax	                          #tear down stacks
popl %ebx
popl %edi
leave
ret

# -------------------------------------User spac------------------------------------------------
# void user_space_alloc:(void);
# Description:  allocate page for the system
#  Interface :  Register-based arguments (not C-style)
#     Inputs :  none
#    Outputs :  allocate spaces for user space
# Registers: gs,fs,es,ds,eax,esp,ebp,edi,esi,edx,ecx,ebx
user_space_alloc:
pushl %ebp                           #construct a stack
movl %esp, %ebp
pushl %edi
pushl %ebx
pushl %eax
movl Program_img,%ebx                #fetch vitual
movl Program_phys,%eax               #prepare the entry physical
movl 8(%ebp),%edi                    #get parameter
addl %edi,%eax                       #8MB+4MB*user_space_alloc
shrl $22,%ebx                        #get index of page dir(higher bits)
andl $0xFFC00000,%eax                #get higher physaddr bits after masked
orl  $0x087, %eax                    #set U/S(bit 2),R/W(bit 1),P(bit 0) and PSE(bit 7) 
leal PageDir,%edi                    #load page dir
movl %eax,(%edi,%ebx,4)              #build the entry
call flush_tlb
popl %eax	                         #tear down stacks
popl %ebx
popl %edi
leave
ret

#---------------------------------------flush tlb-----------------------------------------------
flush_tlb:
pushl %ebp                           #construct a stack
movl %esp, %ebp
pushl %eax
movl %cr3,%eax
movl %eax,%cr3
popl %eax
leave
ret

#---------------------------------------4kb page_mapper-----------------------------------------
user_page_mapper_4K:
pushl %ebp                           #construct a stack
movl %esp, %ebp
pushl %edi
pushl %ebx
pushl %eax
movl 8(%ebp),%ebx                    #fetch vitual
shrl $22,%ebx                        #get index of page dir(higher bits)
xorl %eax, %eax                      #entry for dir
orl $0x07, %eax                      #set U/S(bit 2),R/W(bit 1),P(bit 0)
leal PageTbl,%edi                    #get the page table 
orl %edi,%eax                        #load the table 
leal PageDir,%edi                    #load page dir
movl %eax,(%edi,%ebx,4)              #build the entry
movl 8(%ebp),%ebx                    #fetch vitual
movl 12(%ebp),%eax                   #prepare the entry
shrl $12,%ebx                        #get index of page dir(higher bits)
andl $0x000003FF,%ebx                #get the lower bits as indes of page table
andl $0x003FF000,%eax                #the page physical addr's higher bits
orl $0x07,%eax                       #set U/S(bit 2),R/W(bit 1),P(bit 0)
leal PageTbl,%edi                    #load page dir
movl %eax,(%edi,%ebx,4)              #build the entry
call flush_tlb                       #flush_tlb
popl %eax	                         #tear down stacks
popl %ebx
popl %edi
leave
ret
#---------------------------------------4Mb page_mapper-----------------------------------------
user_page_mapper_4M:
pushl %ebp                           #construct a stack
movl %esp, %ebp
pushl %edi
pushl %ebx
pushl %eax
movl 8(%ebp),%ebx                    #fetch vitual
movl 12(%ebp),%eax                   #prepare the entry physical
shrl $22,%ebx                        #get index of page dir(higher bits)
andl $0xFFC00000,%eax                #get higher physaddr bits after masked
orl  $0x087, %eax                    #set U/S(bit 2),R/W(bit 1),P(bit 0) and PSE(bit 7) 
leal PageDir,%edi                    #load page dir
movl %eax,(%edi,%ebx,4)              #build the entry
call flush_tlb
popl %eax	                         #tear down stacks
popl %ebx
popl %edi
leave
ret

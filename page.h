#ifndef _PAGE_H
#define _PAGE_H
#include "types.h"
#define fourmb 0x0400000

#define CreateNewPageTable(name) \
ALIGN 4096;\
name: \
.rept 1024\
.long 0\
.endr\


extern void paging(void);
extern __attribute__((regparm(0))) void user_space_alloc(uint32_t pid_4mb);
extern void flush_tlb(void);
//////////////////////////////////////////////////////////////MUST READ THE COMMENT!!!!!!//////////////////////////////////////////////////

//Cannot map many 1 virtual page to many physical page at the same time, but can map many 1 physical page to virtual page at the same time  
//UNSAFE function:
//would raise fatal fault when mapping to vir_addr(This can hardly be fixed):
//0x00400000~0x007FFFFF;
//0x08000000~0x083FFFFF;
//
//Essentially:
//IT ONLY WORKS FOR virtual address(This can be fixed by entroducing new Page Table):
//0x00000000~0x003FFFFF
extern __attribute__((regparm(0))) void user_page_mapper_4K(uint32_t vir_addr,uint32_t phys_addr); 
//UNSAFE function:
//would raise fatal fault when mapping to vir_addr(This can hardly be fixed):
//0x00000000~0x003FFFFF;
//
//Essentially:
//IT ONLY WORKS FOR virtual address 
//0x00400000~0x007FFFFF;0x08000000~0x083FFFFF
//which is already allocated (therefore useless)(This can be fixed by entroducing new Page Table)
//Therefore this function is completely useless and should never be used.
extern __attribute__((regparm(0))) void user_page_mapper_4M(uint32_t vir_addr,uint32_t phys_addr);

#endif 






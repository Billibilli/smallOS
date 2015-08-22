#ifndef _IDT_HANDLER_H
#define _IDT_HANDLER_H
#include "types.h"
#include "task.h"
//system internal interrupts and expections
extern void divide_error(void);
extern void debug(void);
extern void nmi(void);
extern void int3(void);
extern void overflow(void);
extern void bounds(void);
extern void invalid_op(void);
extern void device_not_available(void);
extern void doublefault_fn(void);
extern void coprocessor_segment_overrun(void);
extern void invalid_TSS(void);
extern void segment_not_present(void);
extern void stack_segment(void);
extern void general_protection(void);
extern void page_fault(void);
extern void coprocessor_error(void);
extern void alignment_check(void);
extern void machine_check(void);
extern void simd_coprocessor_error(void);
//interrupt of outer devices
extern void do_irq1(void);
extern void do_irq8(void);
extern void do_irq0();
#endif 



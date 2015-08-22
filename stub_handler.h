#ifndef _STUB_HANDLER_H
#define _STUB_HANDLER_H

#define ENTRY(name) \
.globl name; \
ALIGN;\
name:
extern void irq0();
extern void irq1();
extern void irq8();
extern void system_call();

#endif 

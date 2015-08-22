#include "lib.h"
#include "idt_handler.h"
#include "i8259.h"
#include "terminal.h"
#include "rtc.h"
#define nFrequence 404
static int counter =0;
static int flages =0;
static void deep(void);
/* divide_error
 * Description: display divide_error message
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: enable interrups and keep displaying divide_error message
 */
void 
divide_error()
{
sti();
printf("Divide error \n");
while(1);
}
/* debug
 * Description: display debug message
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: enable interrups and keep displaying debug message
 */
void 
debug()
{
sti();
printf("debug \n");
while(1);
}
/* nmi
 * Description: display nmi message
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: enable interrups and keep displaying nmi message
 */
void 
nmi()
{
sti();
printf("nmi \n");
while(1);
}
/* int3
 * Description: display Break point message
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: enable interrups and keep displaying Break point message
 */
void 
int3()
{sti();
printf("Break point \n");
while(1);
}
/* overflow
 * Description: display Overflow message
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: enable interrups and keep displaying Overflow message
 */
void 
overflow()
{
sti();
printf("Overflow \n");
while(1);
}
/* bounds
 * Description: display Bounds check message
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: enable interrups and keep displaying Bounds check message
 */
void 
bounds()
{
sti();
printf("Bounds check \n");
while(1);
}
/* invalid_op
 * Description: display Invalid opcode message
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: enable interrups and keep displaying Invalid opcode message
 */
void 
invalid_op()
{
sti();
printf("Invalid opcode \n");
while(1);
}
/* device_not_available
 * Description: display Device_not_available message
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: enable interrups and keep displaying Device_not_available message
 */
void 
device_not_available()
{
sti();
printf("Device_not_available \n");
while(1);
}
/* doublefault_fn
 * Description: display Double fault message
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: enable interrups and keep displaying Double fault message
 */
void 
doublefault_fn()
{
sti();
printf("Double fault \n");
while(1);
}
/* coprocessor_segment_overrun
 * Description: display coprocessor_segment_overrunt message
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: enable interrups and keep displaying 
 *					  coprocessor_segment_overrun message
 */
void 
coprocessor_segment_overrun()
{
sti();
printf("Coprocessor segment overrun \n");
while(1);
}
/* invalid_TSS
 * Description: display invalid TSS message
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: enable interrups and keep displaying invalid TSS message
 */
void 
invalid_TSS()
{
sti();
printf("invalid TSS \n");
while(1);
}
/* segment_not_present
 * Description: display invalid Segment not present message
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: enable interrups and keep displaying Segment not present message
 */
void 
segment_not_present()
{
sti();
printf("Segment not present \n");
while(1);
}
/* stack_segment
 * Description: display invalid stack_segment message
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: enable interrups and keep displaying stack_segment message
 */
void 
stack_segment()
{
sti();
printf("Stack segment fault \n");
while(1);
}
/* general_protection
 * Description: display invalid general_protection message
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: enable interrups and keep displaying general_protection message
 */
void 
general_protection()
{
sti();
printf("General protection \n");
//zhuxihao added it on 3-24
//open_terminal();
//end
while(1);
}
/* page_fault
 * Description: display invalid page_fault message
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: enable interrups and keep displaying page_fault message
 */
void 
page_fault()
{
sti();
printf("page fault \n");
while(1);
}

/* coprocessor_error
 * Description: display invalid Floating-point error message
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: enable interrups and keep displaying Floating-point error message
 */
void 
coprocessor_error()
{
sti();
printf("Floating-point error \n");
while(1);
}
/* alignment_check
 * Description: display invalid alignment_check message
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: enable interrups and keep displaying alignment_check message
 */
void 
alignment_check()
{
sti();
printf("Alignment check \n");
while(1);
}
/* machine_check
 * Description: display invalid Machine check message
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: enable interrups and keep displaying Machine check message
 */
void 
machine_check()
{
sti();
printf("Machine check \n");
while(1);
}
/* simd_coprocessor_error
 * Description: display SIMD floating point check message
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: enable interrups and keep SIMD floating point check message
 */
void 
simd_coprocessor_error()
{
sti();
printf("SIMD floating point \n");
while(1);
}


//void 
//system_call()
//{
//printf("system_call \n");
//}
/* do_irq1
 * Description: set up irq1 which is the keyboard
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: enable irq1
 */
void 
do_irq1()
{
	//int rec;
	unsigned char scancode = inb(0x60);
	//if(scancode==0x0E || scancode==0x8E) printf("\b");
	//unsigned char key=scancode;
	//if(key<0x60 && key>=0x00)
	//printf("we want %x ", scancode);
	void *p;
	uint32_t fd01;
	read_terminal(fd01, p, scancode);
	
	//outb(0x20, 0x20);
	//send the ending interrupt signal
	send_eoi(1);
	
}
/* do_irq8
 * Description: set up irq1 which is the keyboard
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: enable irq8
 */
void 
do_irq8()
{
	//write to the rtc register
	//printf("rtc ");
	outb(0x0C,0x70);
	inb(0x71);
    happen();	
	//send the ending interrupt signal
	send_eoi(8);
}
/* do_irq0
 * Description: set up irq1 which is the keyboard
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: enable irq10
 */
void
do_irq0()
{

		deep();
       counter++;
        if(counter >= 60)
       {
               flages = 1;
               uint8_t tmp = (inb(0x61) & 0xFC);
               outb( tmp, 0x61);
       }

	//printf("do_irq0");
	switch_task();
	send_eoi(0);
}



void deep(void)
{
		uint32_t Div;
		uint8_t tmp;
       if(flages == 0)
       {
                       //Set the PIT to the desired frequency
               Div = 1193180 / nFrequence;
               outb(0xb6, 0x43);
               outb( (uint8_t)(Div), 0x42 );
               outb( (uint8_t) (Div >> 8), 0x42);

                       // And play the sound using the PC speaker
			   tmp = inb(0x61);
               if (tmp != (tmp | 3)) {
                     outb( tmp | 3, 0x61);}
       }
}


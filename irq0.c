#include "irq0.h"
#include "lib.h"
#include "i8259.h"

//NoBcd,leastfirst then msn, cntr counter0
#define timer_ctrl_port 0x43
#define timer_port 0x40
#define INTR_ON_TERMNAL_COUNT 0x30
#define HARD_RE_TRIG_ONE_SHOT 0x32
#define RATE_GENERATOR 0x34
#define SQUARE_WAVE_MODE 0x36
#define SOFTWARE_STROBE 0x38
#define HZ 20
#define DIVISOR 1193180/HZ
/* open_IRQ0
 * Description: start up the IRQ0
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: enable irq0
 */
void
open_IRQ0(void)
{

outb(RATE_GENERATOR, timer_ctrl_port);
/* Write the divisor to the time chip port*/
outb(DIVISOR&0xFF, timer_port);
outb(DIVISOR>>8, timer_port);
enable_irq(0);
//printf("IRQ0 ENABLED");
}
/* open_IRQ12
 * Description: start up the IRQ12
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: enable irq12
 */
void
open_IRQ12(void)
{

printf("enter mouse");
/*
	frequency approximate 18hz 
*/
//set up the registers.
outb(0xFF, 0x60);
outb(0xF6, 0x60);
outb(0xE6, 0x60);
// enable the mouse.
enable_irq(12);
printf("mouse ENABLED");
}




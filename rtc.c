#include "rtc.h"
#include "lib.h"
#define debug_read_rtc 0
#include "task.h"
static int getrate(int f);
static volatile int itr_flag[NUM_TERMINAL]={0};
extern volatile uint32_t cur_terminal;
extern volatile uint32_t alt_terminal;

/* 
 * open_rtc
 *   Description: turn on the IRQ8
 *	 INPUTS: none			   			 
 *   OUTPUTS: reset register A
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none 
 */
void
open_rtc(void)
{

//setting up the 
outb(0x0B, 0x70);
/* Read the current value of register*/
char x = inb(0x71);
x |= 0x40;
/* Set the index again*/
outb(0x0B, 0x70);
/* Write the previous value*/
outb(x, 0x71);
/* Set the default frequency*/
write_rtc(2);
}

/* 
 * read_rtc
 *   Description: check if the rtc is currently working
 *	 INPUTS: none			   			 
 *   OUTPUTS: reset the itr_flag to 0
 *   RETURN VALUE: 0 for success and -1 for failure
 *   SIDE EFFECTS: none 
 */
int read_rtc(void)
{
#if debug_read_rtc
printf("rtc in\n");
#endif

while(itr_flag[cur_terminal]==0);
itr_flag[cur_terminal]=0;
#if debug_read_rtc
printf("rtc out\n");
#endif
return 0;
}
/* 
 * happen
 *   Description: set the interrupt
 *	 INPUTS: none		   			 
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none 
 */
void 
happen(void)
{
itr_flag[cur_terminal]=1;
}
/* 
 * write_rtc
 *   Description: write the frequency into the register A
 *	 INPUTS: freq -- the frequency			   			 
 *   OUTPUTS: none
 *   RETURN VALUE: 0 for success and -1 for failure
 *   SIDE EFFECTS: none 
 */

int write_rtc(int freq)
{
int rate;
rate=getrate(freq*4);
if(rate==-1) return -1;
rate &= 0x0F;			/* Ensure that rate must be above 2 and not over 15*/
outb(0x0A, 0x70); 		/* Set index to register A*/
char prev=inb(0x71); 	/* Get initial value of register A*/
outb(0x0A, 0x70);		/* Reset index to A*/
/* Write only our rate to A. Note, rate is the lower 4 bits.*/
outb((prev&0xF0)|rate, 0x71); 
return 0;

}
/* 
 * getrate
 *   Description: change the rate to the desired format
 *	 INPUTS: f -- the frequency			   			 
 *   OUTPUTS: none
 *   RETURN VALUE: the modified frequency
 *   SIDE EFFECTS: none 
 */

int getrate(int f)
{
int rate;
switch(f)
{
case 2:
 rate=0x000F;
 break;
case 4: 
 rate=0x000E;
 break;
case 8:
 rate=0x000D;
 break;
 
 case 16:
 rate=0x000C;
 break;
case 32: 
 rate=0x000B;
 break;
case 64:
 rate=0x000A;
 break;
 
 case 128:
 rate=0x0009;
 break;
case 256: 
 rate=0x0008;
 break;
case 512:
 rate=0x0007;
 break;
 
case 1024:
 rate=0x0006;
 break;
case 2048: 
 rate=0x0005;
 break;
case 4096:
 rate=0x0004; 
 break;
case 8192:
 rate=0x0003;
 break;
 
default:
  rate=-1;
 
}
return rate;
}



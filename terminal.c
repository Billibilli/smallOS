#include "terminal.h"
#include "lib.h"
#include "task.h"
#include "table.h"

static int ctrl=0;
static int alt=0;
static int capital=0;

static int counter[NUM_TERMINAL];
static volatile int lock[NUM_TERMINAL];
static char buffer[NUM_TERMINAL][1024];

static void command(unsigned char cmd);
static void common(unsigned char key);
static void transform(unsigned key);
static void clear_buf(void);
static void up_buf(void);
static void down_buf(void);

/* bufdel
 * Description: delete the buffer 
 * 		 Input: N/A
 *       Output: decrement the counter
 *		 Return value: -1 if failed and 1 if succeed
 * 		 Side effect:  N/A
 */
static int bufdel(void)
{
if(counter[alt_terminal] == 0)
	return -1;
		// decrement the counter
		counter[alt_terminal]--;
		// set the buffer to 0
		buffer[alt_terminal][counter[alt_terminal]]=0;
	return 1;
}
/* clear_buf
 * Description: clear the buffer
 * 		 Input: N/A
 *       Output: clear the buffer
 *		 Return value: N/A
 * 		 Side effect:  N/A
 */

void clear_buf(void)
{
	int j;
	  for(j=0;j< counter[alt_terminal];j++)
	   /*Clear every elements in this buffer */
		 {buffer[alt_terminal][j]=0;}
	/* Counter goes to 0*/			
	counter[alt_terminal]=0;	 
}
/* command
 * Description: check the command
 * 		 Input: --cmd: command input 
 *       Output: clear N/A
 *		 Return value: N/A
 * 		 Side effect:  open the terminal
 */


void command(unsigned char cmd)
{
	uint8_t file;
	if(cmd==0x26) open_terminal(&file);
}
/* altcommand
 * Description: check the command
 * 		 Input: --cmd: command input 
 *       Output: clear N/A
 *		 Return value: N/A
 * 		 Side effect:  change the task
 */
void altcommand(unsigned char cmd)
{	
	
	if(cmd==0x3c) 
	{
		changetask();
	}
}

/* common
 * Description: deal with the common key input without capitals.
 * 		 Input: --key: key scanned from keyboard 
 *       Output: clear N/A
 *		 Return value: N/A
 * 		 Side effect:  output the char or turn on capital. 
 */
void common(unsigned char key)
{
//printf("key : %x ::",key);
switch(key)
{
case 0x1C: 	putcur('\n');
			// set the lock
			lock[alt_terminal] = 1;
			buffer[alt_terminal][counter[alt_terminal]]= '\n';
			// increment the counter
			counter[alt_terminal]++;
			//screen_ptr(getx(), gety());
			//printf("X: %d,Y: %d",getx(), gety());
			break;         // ENTER!!!!!!!!!!  		   
// "tab"			
case 0x0F: printf("%c", 9); break;


// back space
case 0x0E: 
		
        if(bufdel() == 1 )
        {
			bksp_t();
			break;
		}
		else
		break;

//direction 
case 0x50:	down_buf();break;//down
case 0x48:  up_buf();break;//up
case 0x4D: dir(key);break;
case 0x4B: dir(key);break;

}
}
/* up_buf
 * Description: increse the buffer
 * 		 Input: N/A
 *       Output:  N/A
 *		 Return value: N/A
 * 		 Side effect: modify the counter
 */
void up_buf(void)
{
	int i;
	for (i = 0; i < strlen(buffer[alt_terminal]); i++)
	{
		bksp_t();
	}
	counter[alt_terminal] = uptable((char * )buffer[alt_terminal],alt_terminal);//down
	for (i = 0; i < strlen(buffer[alt_terminal]); i++)
	{
		putcur(buffer[alt_terminal][i]);
	}
}
/* down_buf
 * Description: decrease the buffer
 * 		 Input: N/A
 *       Output: clear N/A
 *		 Return value: N/A
 * 		 Side effect: modify the counter
 */
void down_buf(void)
{	
	int i;
	for (i = 0; i < strlen(buffer[alt_terminal]); i++)
	{
		bksp_t();
	}
	
	counter[alt_terminal] = downtable((char * )buffer[alt_terminal],alt_terminal);//up
	
	for (i = 0; i < strlen(buffer[alt_terminal]); i++)
	{
		putcur(buffer[alt_terminal][i]);
	}
	
}
/* open_terminal
 * Description: Turn on the terminal
 * 		 Input: --filename: file need to access
 *       Output: clear N/A
 *		 Return value: N/A
 * 		 Side effect: clear
 */
void 
open_terminal(const uint8_t* filename)
{
//int8_t s[9]="[user@ ]$";
clear();

clear_buf();

}

/* read_terminal
 * Description: read the keyboard input
 * 		 Input: --fd: file descriptor
 *			    --buf: 
 *				--key : scan code for keyboard
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: transform the keys to Capital if needed
 */

void read_terminal(int32_t fd, const void* buf, int32_t key)
{
	common(key); //common parts, don't need any Capital letter consideration:D

	//to judge the CAPITAL or NOT
	if(key==0x2A ||key==0x36) 
		capital++; //shift
	else if(key==0xAA || key==0xB6) 	
		capital--;

	if(key==0x3A)
		capital=(capital==0)?1:0; //Capslock[cur_terminal]


	//to judge the ctrl
	if(key==0x1D) 
		ctrl++;
	else 
		if(key==0x9D) ctrl--;
	//to judge the alt	
	if(key==0x38) 
		alt++;
	else 
		if(key==0xB8) alt--;
	
	if(alt!=0)
		altcommand(key);
	else if(ctrl!=0)
		command(key);  
	else if(key<0x60 && key>=0x00)
		{
		// change to capital letters
		transform(key);
		}
		
}
/* write_terminal
 * Description: write the keyboard input
 * 		 Input: --fd: file descriptor
 *			    --buf: buf need to print
 *				--nbytes : number of bytes need to write
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: print to the current buffer
 */

void write_terminal(int32_t fd, void* buf, int32_t nbytes)
{
	printscur((char *) buf);
}

/* r_terminal
 * Description: read the terminal when switch terminals
 * 		 Input: --fd: file descriptor
 *			    --buf: buf need to print
 *				--key : scan code for keyboard
 *       Output: N/A
 *		 Return value: k: counter
 * 		 Side effect: print to the current buffer
 */

int r_terminal(int32_t fd, uint8_t* buf, int32_t key)
{

	int i,k;
	//set the lock to 0
	lock[alt_terminal]=0;
	while(1){ 
		// only return the keyboard input after press "Enter"
		if((lock[alt_terminal] == 1 ) && (alt_terminal == cur_terminal))
		break;}
	//set the lock to 0		
	lock[alt_terminal]=0;
	for(i =0; i < key; i++)
		// store them into the buffer
		buf[i] = (uint8_t) buffer[alt_terminal][i];
		k = counter[alt_terminal];
	cli();
	pushtable((char *) buf, alt_terminal);
	sti();
	// clear the buffer	
	clear_buf();
	return k;
}


/* transform
 * Description: transform the letter to capital
 * 		 Input: --key : scan code for keyboard
 *       Output: temp storing the char
 *		 Return value: k: counter
 * 		 Side effect: print the temp
 */


void transform(unsigned key)
{
	char temp;
	int cao=0;
	//small case
	if(capital==0){
switch(key)
{
//1-0
case 0x02: temp='1';  putcur(temp);break;
case 0x03: temp='2';  putcur(temp);break;
case 0x04: temp='3';  putcur(temp);break;
case 0x05: temp='4';  putcur(temp);break;
case 0x06: temp='5';  putcur(temp);break;
case 0x07: temp='6';  putcur(temp);break;
case 0x08: temp='7';  putcur(temp);break;
case 0x09: temp='8';  putcur(temp);break;
case 0x0A: temp='9';  putcur(temp);break;
case 0x0B: temp='0';  putcur(temp); break;
case 0x0C: temp='-';  putcur(temp);break;
case 0x0D: temp='=';  putcur(temp);break;
//q-? only chars
case 0x10: temp='q';  putcur(temp);break; case 0x11: temp='w';  putcur(temp);break; case 0x12: temp='e'; putcur(temp); break;
case 0x13: temp='r';  putcur(temp);break; case 0x14: temp='t'; putcur(temp); break; case 0x15: temp='y';  putcur(temp);break;
case 0x16: temp='u';  putcur(temp);break; case 0x17: temp='i'; putcur(temp); break; case 0x18: temp='o'; putcur(temp); break;
case 0x19: temp='p';  putcur(temp);break; case 0x1A: temp='['; putcur(temp); break; case 0x1B: temp=']'; putcur(temp); break;
case 0x2B: temp=92;   putcur(temp);break;

case 0x1E: temp='a';  putcur(temp);break; case 0x1F: temp='s'; putcur(temp); break; case 0x20: temp='d'; putcur(temp); break;
case 0x21: temp='f';  putcur(temp);break; case 0x22: temp='g'; putcur(temp);break; case 0x23:  temp='h'; putcur(temp);break;
case 0x24: temp='j';  putcur(temp);break; case 0x25: temp='k';  putcur(temp);break; case 0x26: temp='l'; putcur(temp);break;
case 0x27: temp=';';  putcur(temp);break; case 0x28: temp=39;  putcur(temp);break; 

case 0x2C: temp='z'; putcur(temp); break; case 0x2D: temp='x'; putcur(temp); break; case 0x2E: temp='c'; putcur(temp); break;
case 0x2F: temp='v'; putcur(temp); break; case 0x30: temp='b'; putcur(temp); break; case 0x31: temp='n'; putcur(temp); break;
case 0x32: temp='m'; putcur(temp); break; case 0x33: temp=',';  putcur(temp);break; case 0x34: temp='.'; putcur(temp); break;
case 0x35: temp='/'; putcur(temp); break; 
case 0x39: temp=' '; putcur(temp); break;
 default: cao=1;break;

}

}

//CAPITAL case
else
{
switch(key){

case 0x02: temp='!';  putcur(temp);break;
case 0x03: temp='@';  putcur(temp);break;
case 0x04: temp='#';  putcur(temp);break;
case 0x05: temp='$';  putcur(temp);break;
case 0x06: temp='%';  putcur(temp);break;
case 0x07: temp='^';  putcur(temp);break;
case 0x08: temp='&';  putcur(temp);break;
case 0x09: temp='*';  putcur(temp);break;
case 0x0A: temp='(';  putcur(temp);break;
case 0x0B: temp=')';  putcur(temp);break;
case 0x0C: temp='_';  putcur(temp);break;
case 0x0D: temp='+';  putcur(temp);break;
case 0x10: temp='Q';  putcur(temp);break; case 0x11: temp='W';  putcur(temp);break; case 0x12: temp='E';  putcur(temp);break;
case 0x13: temp='R';  putcur(temp);break; case 0x14: temp='T';  putcur(temp);break; case 0x15: temp='Y'; putcur(temp); break;
case 0x16: temp='U';  putcur(temp);break; case 0x17: temp='I';  putcur(temp);break; case 0x18: temp='O'; putcur(temp); break;
case 0x19: temp='P';  putcur(temp);break; case 0x1A: temp='{';  putcur(temp);break; case 0x1B: temp='}'; putcur(temp); break;
case 0x2B: temp='|';  putcur(temp);break;

case 0x1E: temp='A'; putcur(temp);break; case 0x1F: temp='S'; putcur(temp); break; case 0x20: temp='D';  putcur(temp);break;
case 0x21: temp='F'; putcur(temp);break; case 0x22: temp='G'; putcur(temp); break; case 0x23: temp='H';  putcur(temp);break;
case 0x24: temp='J'; putcur(temp);break; case 0x25: temp='K'; putcur(temp); break; case 0x26: temp='L'; putcur(temp); break;
case 0x27: temp=':'; putcur(temp);break; case 0x28: temp=34; putcur(temp); break; 

case 0x2C: temp='Z'; putcur(temp);break; case 0x2D: temp='X'; putcur(temp); break; case 0x2E: temp='C';  putcur(temp);break;
case 0x2F: temp='V'; putcur(temp);break; case 0x30: temp='B'; putcur(temp);break; case 0x31: temp='N'; putcur(temp); break;
case 0x32: temp='M'; putcur(temp);break; case 0x33: temp='<'; putcur(temp); break; case 0x34: temp='>';  putcur(temp);break;
case 0x35: temp='?'; putcur(temp);break; 
case 0x39: temp=' '; putcur(temp);break;
default: cao=1;break;
putcur(temp);
}
}

	if(cao!=1)
	{
			//check if it is out of range
			if(counter[alt_terminal]<1024)
			{
				//restore the value from temp
				buffer[alt_terminal][counter[alt_terminal]]=temp;
				counter[alt_terminal]++;
			}
			else
				printf("out of range");

	}
}






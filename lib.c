/* lib.c - Some basic library functions (printf, strlen, etc.)
 * vim:ts=4 noexpandtab
 */

#include "lib.h"
#define VIDEO 0xB8000
#define NUM_COLS 80
#define NUM_ROWS 25
#define ATTRIB 0x7
#define screen_width 79
#include "task.h"


extern volatile uint32_t cur_terminal;
extern volatile uint32_t alt_terminal;
extern volatile uint32_t system_init_flag;
int screen_x[NUM_TERMINAL];
int screen_y[NUM_TERMINAL];
static char* video_mem = (char *)VIDEO;

  /*dir
  * Description: update the position by direction 
  * 	  Input: --dd: scanned value for direction
  *      Output: N/A
  * Return value: N/A
  * Side effect: update the screen coordinates
  */
void dir(unsigned char dd)
{
	switch(dd)
	{
	// up
	case 0x48: if(screen_y[cur_terminal]!=0) screen_y[cur_terminal]--; break;
	// left
	case 0x4B: if(screen_x[cur_terminal]!=0) screen_x[cur_terminal]--; update_cursor(screen_y[alt_terminal], screen_x[alt_terminal]);break;
	// down
	case 0x50:  screen_y[cur_terminal]++; break;
	// right
	case 0x4D: if(screen_x[cur_terminal]<screen_width) screen_x[cur_terminal]++; 
				else {screen_x[cur_terminal]=0;screen_y[cur_terminal]++;} 
				update_cursor(screen_y[alt_terminal], screen_x[alt_terminal]);
				break;
	}
}
  /*up_screen
  * Description: update the screen 
  * 	  Input: N/A
  *      Output: N/A
  * Return value: N/A
  * Side effect: update the screen coordinates
  */

void up_screen(void)
{
	// if it reaches the right most
	if(screen_x[cur_terminal]>=79)
	{
		// set it to 0
		screen_x[cur_terminal]=0;
		// update the y coordinate
		screen_y[cur_terminal]++;
	}

}


  /*scroll
  * Description: scroll the screen 
  * 	  Input: N/A
  *      Output: N/A
  * Return value: N/A
  * Side effect: update the video mem
  */

void scroll(void)
{
    int32_t i;

	video_mem=(char *)VIDEO;
	for(i=NUM_COLS; i< (NUM_ROWS*NUM_COLS-NUM_COLS); i++) {
	// if it does not reach the bottom line, shift the video mem
        *(uint8_t *)(video_mem + (i << 1)) = *(uint8_t *)(video_mem + ((i+NUM_COLS)<< 1));
		// also the content of color 
        *(uint8_t *)(video_mem + (i << 1) + 1) = *(uint8_t *)(video_mem + ((i+NUM_COLS)<< 1)+1);;
    }
	//if it reaches the bottom line
	for(i=(NUM_ROWS*NUM_COLS-NUM_COLS); i< (NUM_ROWS*NUM_COLS); i++) {
	// replace with space
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
	// as well as the color 	
		*(uint8_t *)(video_mem + (i << 1) + 1) = ATTRIB;;
    }
	
	
	screen_x[cur_terminal] = 0;
	screen_y[cur_terminal]= NUM_ROWS - 1;

}
  /*clear
  * Description: clear the screen 
  * 	  Input: N/A
  *      Output: N/A
  * Return value: N/A
  * Side effect: update the video mem
  */
void
clear(void)
{
    int32_t i;

	video_mem=(char *)VIDEO;
	// fill every pixel in the terminal with space
    for(i=NUM_COLS; i<NUM_ROWS*NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = ATTRIB;;
    }

	
	if(system_init_flag) {
	// mapping the video mem
	video_mem=(char *)VIDEO+(alt_terminal+1)*size_of_vmem;
	// then clear the screen 
    for(i=NUM_COLS; i<NUM_ROWS*NUM_COLS; i++) {
        *(uint8_t *)(video_mem + (i << 1)) = ' ';
        *(uint8_t *)(video_mem + (i << 1) + 1) = ATTRIB;;
    }
	
	}
	screen_x[cur_terminal]=0;
	screen_y[cur_terminal]=1;

}

/* Standard printf().
 * Only supports the following format strings:
 * %%  - print a literal '%' character
 * %x  - print a number in hexadecimal
 * %u  - print a number as an unsigned integer
 * %d  - print a number as a signed integer
 * %c  - print a character
 * %s  - print a string
 * %#x - print a number in 32-bit aligned hexadecimal, i.e.
 *       print 8 hexadecimal digits, zero-padded on the left.
 *       For example, the hex number "E" would be printed as
 *       "0000000E".
 *       Note: This is slightly different than the libc specification
 *       for the "#" modifier (this implementation doesn't add a "0x" at
 *       the beginning), but I think it's more flexible this way.
 *       Also note: %x is the only conversion specifier that can use
 *       the "#" modifier to alter output.
 * */
int32_t
printf(int8_t *format, ...)
{
	
	/* Pointer to the format string */
	int8_t* buf = format;

	/* Stack pointer for the other parameters */
	int32_t* esp = (void *)&format;
	esp++;

	while(*buf != '\0') {
		switch(*buf) {
			case '%':
				{
					int32_t alternate = 0;
					buf++;

format_char_switch:
					/* Conversion specifiers */
					switch(*buf) {
						/* Print a literal '%' character */
						case '%':
							putc('%');
							break;

						/* Use alternate formatting */
						case '#':
							alternate = 1;
							buf++;
							/* Yes, I know gotos are bad.  This is the
							 * most elegant and general way to do this,
							 * IMHO. */
							goto format_char_switch;

						/* Print a number in hexadecimal form */
						case 'x':
							{
								int8_t conv_buf[64];
								if(alternate == 0) {
									itoa(*((uint32_t *)esp), conv_buf, 16);
									if(alt_terminal == cur_terminal)
									printscur(conv_buf);
									else
									puts(conv_buf);
								} else {
									int32_t starting_index;
									int32_t i;
									itoa(*((uint32_t *)esp), &conv_buf[8], 16);
									i = starting_index = strlen(&conv_buf[8]);
									while(i < 8) {
										conv_buf[i] = '0';
										i++;
									}
								if(alt_terminal == cur_terminal)
									printscur(&conv_buf[starting_index]);
									else
									puts(&conv_buf[starting_index]);
								}
								esp++;
							}
							break;

						/* Print a number in unsigned int form */
						case 'u':
							{
								int8_t conv_buf[36];
								itoa(*((uint32_t *)esp), conv_buf, 10);
									if(alt_terminal == cur_terminal)
									printscur(conv_buf);
									else
									puts(conv_buf);
								esp++;
							}
							break;

						/* Print a number in signed int form */
						case 'd':
							{
								int8_t conv_buf[36];
								int32_t value = *((int32_t *)esp);
								if(value < 0) {
									conv_buf[0] = '-';
									itoa(-value, &conv_buf[1], 10);
								} else {
									itoa(value, conv_buf, 10);
								}
									if(alt_terminal == cur_terminal)
									printscur(conv_buf);
									else
									puts(conv_buf);
								esp++;
							}
							break;

						/* Print a single character */
						case 'c':
							putc( (uint8_t) *((int32_t *)esp) );
							esp++;
							break;

						/* Print a NULL-terminated string */
						case 's':
							if(alt_terminal == cur_terminal)
							printscur(*((int8_t **)esp) );
							else
							puts( *((int8_t **)esp) );
							esp++;
							break;

						default:
							break;
					}

				}
				break;

			default:
				putc(*buf);
				break;
		}
		buf++;
	}
	return (buf - format);
}

/* Output a string to the console */
int32_t
puts(int8_t* s)
{
	
	register int32_t index = 0;
	while(s[index] != '\0') {
		putc(s[index]);
		index++;
	}

	return index;
}

void
putc(uint8_t c)
{
	uint32_t i;
	 video_mem=(char *)VIDEO;
		if(c == '\n' || c == '\r') {
		    screen_y[cur_terminal]++;
			screen_x[cur_terminal]=0;
			if(screen_y[cur_terminal] >= 24)
				 {
					for(i=NUM_COLS; i< (NUM_ROWS*NUM_COLS-NUM_COLS); i++) {
					
						*(uint8_t *)(VIDEO + (i << 1)) = *(uint8_t *)(VIDEO+ ((i+NUM_COLS)<< 1));
						*(uint8_t *)(VIDEO+ (i << 1) + 1) = *(uint8_t *)(VIDEO+ ((i+NUM_COLS)<< 1)+1);;
					}
					for(i=(NUM_ROWS*NUM_COLS-NUM_COLS); i< (NUM_ROWS*NUM_COLS); i++) {
						*(uint8_t *)(VIDEO+ (i << 1)) = ' ';
						*(uint8_t *)(VIDEO + (i << 1) + 1) = ATTRIB;
					}
				 	 screen_x[cur_terminal] =0;
					 screen_y[cur_terminal] =23;
				 }
		}
		else
		{
			 if(screen_x[cur_terminal]>=79)
			 {
				 screen_x[cur_terminal]=0;
				 screen_y[cur_terminal]++;
			 }
			*(uint8_t *)(video_mem + ((NUM_COLS*screen_y[cur_terminal] + screen_x[cur_terminal]) << 1)) = c;
			*(uint8_t *)(video_mem + ((NUM_COLS*screen_y[cur_terminal] + screen_x[cur_terminal]) << 1) + 1) = ATTRIB;
			 screen_x[cur_terminal]++;
			 screen_x[cur_terminal] %= NUM_COLS;
			if(screen_y[cur_terminal] >= 24)
				 {
					for(i=NUM_COLS; i< (NUM_ROWS*NUM_COLS-NUM_COLS); i++) {
					
						*(uint8_t *)(VIDEO + (i << 1)) = *(uint8_t *)(VIDEO+ ((i+NUM_COLS)<< 1));
						*(uint8_t *)(VIDEO+ (i << 1) + 1) = *(uint8_t *)(VIDEO+ ((i+NUM_COLS)<< 1)+1);;
					}
					for(i=(NUM_ROWS*NUM_COLS-NUM_COLS); i< (NUM_ROWS*NUM_COLS); i++) {
						*(uint8_t *)(VIDEO+ (i << 1)) = ' ';
						*(uint8_t *)(VIDEO + (i << 1) + 1) = ATTRIB;
					}
				 	 screen_x[cur_terminal] =0;
					 screen_y[cur_terminal] =23;
				 }
		}

if(system_init_flag)
	if(cur_terminal == alt_terminal)
	{
			// *(uint8_t *)(VIDEO+(alt_terminal+1)*size_of_vmem + ((NUM_COLS*screen_y[cur_terminal] + screen_x[cur_terminal]) << 1)) = c;
			// *(uint8_t *)(VIDEO+(alt_terminal+1)*size_of_vmem + ((NUM_COLS*screen_y[cur_terminal] + screen_x[cur_terminal]) << 1) + 1) = ATTRIB;
			memcpy((char *)VIDEO+(alt_terminal+1)*size_of_vmem	,(char *)VIDEO, size_of_vmem);
			update_cursor(screen_y[alt_terminal], screen_x[alt_terminal]);
	}


}

/* Convert a number to its ASCII representation, with base "radix" */
int8_t*
itoa(uint32_t value, int8_t* buf, int32_t radix)
{
	static int8_t lookup[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	int8_t *newbuf = buf;
	int32_t i;
	uint32_t newval = value;

	/* Special case for zero */
	if(value == 0) {
		buf[0]='0';
		buf[1]='\0';
		return buf;
	}

	/* Go through the number one place value at a time, and add the
	 * correct digit to "newbuf".  We actually add characters to the
	 * ASCII string from lowest place value to highest, which is the
	 * opposite of how the number should be printed.  We'll reverse the
	 * characters later. */
	while(newval > 0) {
		i = newval % radix;
		*newbuf = lookup[i];
		newbuf++;
		newval /= radix;
	}

	/* Add a terminating NULL */
	*newbuf = '\0';

	/* Reverse the string and return */
	return strrev(buf);
}

/* In-place string reversal */
int8_t*
strrev(int8_t* s)
{
	register int8_t tmp;
	register int32_t beg=0;
	register int32_t end=strlen(s) - 1;

	while(beg < end) {
		tmp = s[end];
		s[end] = s[beg];
		s[beg] = tmp;
		beg++;
		end--;
	}

	return s;
}

/* String length */
uint32_t
strlen(const int8_t* s)
{
	register uint32_t len = 0;
	while(s[len] != '\0')
		len++;

	return len;
}

/* Optimized memset */
void*
memset(void* s, int32_t c, uint32_t n)
{
	c &= 0xFF;
	asm volatile("                  \n\
			.memset_top:            \n\
			testl   %%ecx, %%ecx    \n\
			jz      .memset_done    \n\
			testl   $0x3, %%edi     \n\
			jz      .memset_aligned \n\
			movb    %%al, (%%edi)   \n\
			addl    $1, %%edi       \n\
			subl    $1, %%ecx       \n\
			jmp     .memset_top     \n\
			.memset_aligned:        \n\
			movw    %%ds, %%dx      \n\
			movw    %%dx, %%es      \n\
			movl    %%ecx, %%edx    \n\
			shrl    $2, %%ecx       \n\
			andl    $0x3, %%edx     \n\
			cld                     \n\
			rep     stosl           \n\
			.memset_bottom:         \n\
			testl   %%edx, %%edx    \n\
			jz      .memset_done    \n\
			movb    %%al, (%%edi)   \n\
			addl    $1, %%edi       \n\
			subl    $1, %%edx       \n\
			jmp     .memset_bottom  \n\
			.memset_done:           \n\
			"
			:
			: "a"(c << 24 | c << 16 | c << 8 | c), "D"(s), "c"(n)
			: "edx", "memory", "cc"
			);

	return s;
}

/* Optimized memset_word */
void*
memset_word(void* s, int32_t c, uint32_t n)
{
	asm volatile("                  \n\
			movw    %%ds, %%dx      \n\
			movw    %%dx, %%es      \n\
			cld                     \n\
			rep     stosw           \n\
			"
			:
			: "a"(c), "D"(s), "c"(n)
			: "edx", "memory", "cc"
			);

	return s;
}

/* Optimized memset_dword */
void*
memset_dword(void* s, int32_t c, uint32_t n)
{
	asm volatile("                  \n\
			movw    %%ds, %%dx      \n\
			movw    %%dx, %%es      \n\
			cld                     \n\
			rep     stosl           \n\
			"
			:
			: "a"(c), "D"(s), "c"(n)
			: "edx", "memory", "cc"
			);

	return s;
}

/* Optimized memcpy */
void*
memcpy(void* dest, const void* src, uint32_t n)
{
	asm volatile("                  \n\
			.memcpy_top:            \n\
			testl   %%ecx, %%ecx    \n\
			jz      .memcpy_done    \n\
			testl   $0x3, %%edi     \n\
			jz      .memcpy_aligned \n\
			movb    (%%esi), %%al   \n\
			movb    %%al, (%%edi)   \n\
			addl    $1, %%edi       \n\
			addl    $1, %%esi       \n\
			subl    $1, %%ecx       \n\
			jmp     .memcpy_top     \n\
			.memcpy_aligned:        \n\
			movw    %%ds, %%dx      \n\
			movw    %%dx, %%es      \n\
			movl    %%ecx, %%edx    \n\
			shrl    $2, %%ecx       \n\
			andl    $0x3, %%edx     \n\
			cld                     \n\
			rep     movsl           \n\
			.memcpy_bottom:         \n\
			testl   %%edx, %%edx    \n\
			jz      .memcpy_done    \n\
			movb    (%%esi), %%al   \n\
			movb    %%al, (%%edi)   \n\
			addl    $1, %%edi       \n\
			addl    $1, %%esi       \n\
			subl    $1, %%edx       \n\
			jmp     .memcpy_bottom  \n\
			.memcpy_done:           \n\
			"
			:
			: "S"(src), "D"(dest), "c"(n)
			: "eax", "edx", "memory", "cc"
			);

	return dest;
}

/* Optimized memmove (used for overlapping memory areas) */
void*
memmove(void* dest, const void* src, uint32_t n)
{
	asm volatile("                  \n\
			movw    %%ds, %%dx      \n\
			movw    %%dx, %%es      \n\
			cld                     \n\
			cmp     %%edi, %%esi    \n\
			jae     .memmove_go     \n\
			leal    -1(%%esi, %%ecx), %%esi    \n\
			leal    -1(%%edi, %%ecx), %%edi    \n\
			std                     \n\
			.memmove_go:            \n\
			rep     movsb           \n\
			"
			:
			: "D"(dest), "S"(src), "c"(n)
			: "edx", "memory", "cc"
			);

	return dest;
}

/* Standard strncmp */
int32_t
strncmp(const int8_t* s1, const int8_t* s2, uint32_t n)
{
	int32_t i;
	for(i=0; i<n; i++) {
		if( (s1[i] != s2[i]) ||
				(s1[i] == '\0') /* || s2[i] == '\0' */ ) {

			/* The s2[i] == '\0' is unnecessary because of the short-circuit
			 * semantics of 'if' expressions in C.  If the first expression
			 * (s1[i] != s2[i]) evaluates to false, that is, if s1[i] ==
			 * s2[i], then we only need to test either s1[i] or s2[i] for
			 * '\0', since we know they are equal. */

			return s1[i] - s2[i];
		}
	}
	return 0;
}

/* Standard strcpy */
int8_t*
strcpy(int8_t* dest, const int8_t* src)
{
	int32_t i=0;
	while(src[i] != '\0') {
		dest[i] = src[i];
		i++;
	}

	dest[i] = '\0';
	return dest;
}

/* Standard strncpy */
int8_t*
strncpy(int8_t* dest, const int8_t* src, uint32_t n)
{
	int32_t i=0;
	while(src[i] != '\0' && i < n) {
		dest[i] = src[i];
		i++;
	}

	while(i < n) {
		dest[i] = '\0';
		i++;
	}

	return dest;
}

void
test_interrupts(void)
{

	int32_t i;

	video_mem=(char *)VIDEO;
		for (i=0; i < NUM_ROWS*NUM_COLS; i++) {
			video_mem[i<<1]++;
	}
	if(system_init_flag) {
	video_mem=(char *)VIDEO+(alt_terminal+1)*size_of_vmem;
	for (i=0; i < NUM_ROWS*NUM_COLS; i++) {
		video_mem[i<<1]++;
	}
	}
}

void 
screen_ptr(int x, int y)
{
		screen_x[alt_terminal]=x;
		screen_y[alt_terminal]=y;
		update_cursor(y, x);
}


int gety()
{
		return screen_y[cur_terminal];
}

int getx()
{
		return screen_x[cur_terminal];
}

 /*bksp
  * Description: back space function
  * 	  Input: N/A
  *      Output: N/A
  * Return value: N/A
  * Side effect: update the screen coordinate
  */
void bksp(void)
{
	// if not from the starting point, update position and replace with space
		if(screen_x[alt_terminal]!=0)
		{
				screen_x[alt_terminal]--;
			printf(" ");
			screen_x[alt_terminal]--;
	//make sure not delete the status bar 		
		}
		else if(screen_y[alt_terminal]!=0)
		{
			screen_y[alt_terminal]--;
			screen_x[alt_terminal] = 79;
		
		}
	// update the cursor	
	update_cursor(screen_y[alt_terminal], screen_x[alt_terminal]);
}

 /*bksp_t
  * Description: delete the keyboard input
  * 	  Input: N/A
  *      Output: N/A
  * Return value: N/A
  * Side effect: modify the video mem
  */

void
bksp_t(void)
{
	// move the actal cursor
	screen_x[alt_terminal]--;
	// replace the present value with space
	*(uint8_t *)(VIDEO + ((NUM_COLS*screen_y[alt_terminal] + screen_x[alt_terminal]) << 1)) = ' ';
	// update the foreground color. 
	*(uint8_t *)(VIDEO + ((NUM_COLS*screen_y[alt_terminal] + screen_x[alt_terminal]) << 1) + 1) = ATTRIB;
	*(uint8_t *)(VIDEO+(alt_terminal+1)*size_of_vmem + ((NUM_COLS*screen_y[alt_terminal] + screen_x[alt_terminal]) << 1)) = ' ';
	*(uint8_t *)(VIDEO+(alt_terminal+1)*size_of_vmem + ((NUM_COLS*screen_y[alt_terminal] + screen_x[alt_terminal]) << 1) + 1) = ATTRIB;
	
	update_cursor(screen_y[alt_terminal], screen_x[alt_terminal]);

}


void update_cursor(int row, int col)
 {
    unsigned short position=(row*80) + col;
    // cursor LOW port to vga INDEX register
    outb(0x0F, 0x3D4);
    outb((unsigned char)(position&0xFF), 0x3D5);
    // cursor HIGH port to vga INDEX register
    outb(0x0E, 0x3D4);
    outb((unsigned char )((position>>8)&0xFF), 0x3D5);
 }
 /*statusbar
  * Description: display status
  * 	  Input: N/A
  *      Output: N/A
  * Return value: N/A
  * Side effect: modify the video mem
  */
 void statusbar(void)
{
	int i,j;
	uint8_t text[] = "terminal                                                         IOTA OS       ";
	text[9] = (uint8_t) 49;
	//mapping the video mem from text
	video_mem=(char *)VIDEO;
		for(i=0; i< 80 ; i++) {

			*(uint8_t *)(video_mem + (i << 1)) = text[i];
			*(uint8_t *)(video_mem + (i << 1) + 1) = 0x3 | 0x80;
		}
	
	
	for(j =0; j< NUM_TERMINAL; j++)
	{
	// text[9] indicating the terminal number
		text[9] = (uint8_t) j + 49;
		video_mem=(char *)VIDEO+(j+1)*size_of_vmem;
		for(i=0; i< 80 ; i++) {
		// the video mem will never interact with alt_terminal.
			*(uint8_t *)(video_mem + (i << 1)) = text[i];
			*(uint8_t *)(video_mem + (i << 1) + 1) = 0x3 | (0x80 + 0x10*j);
		}
		screen_x[j]=0;
		screen_y[j]=1;
	}


}
  /*putcur
  * Description: print current terminal to the screen
  * 	  Input: --c: command
  *      Output: N/A
  * Return value: N/A
  * Side effect: N/A
  */
void putcur(char c)
{

		int32_t i;
		if(c == '\n' || c == '\r') {//change line
		    screen_y[alt_terminal]++;
			screen_x[alt_terminal]=0;
			if(screen_y[alt_terminal] >= 24)//scolling
				 {
					for(i=NUM_COLS; i< (NUM_ROWS*NUM_COLS-NUM_COLS); i++) {
					
						*(uint8_t *)(VIDEO+(alt_terminal+1)*size_of_vmem + (i << 1)) = *(uint8_t *)(VIDEO+(alt_terminal+1)*size_of_vmem + ((i+NUM_COLS)<< 1));
						*(uint8_t *)(VIDEO+(alt_terminal+1)*size_of_vmem + (i << 1) + 1) = *(uint8_t *)(VIDEO+(alt_terminal+1)*size_of_vmem + ((i+NUM_COLS)<< 1)+1);;
					}
					for(i=(NUM_ROWS*NUM_COLS-NUM_COLS); i< (NUM_ROWS*NUM_COLS); i++) {
						*(uint8_t *)(VIDEO+(alt_terminal+1)*size_of_vmem + (i << 1)) = ' ';
						*(uint8_t *)(VIDEO+(alt_terminal+1)*size_of_vmem + (i << 1) + 1) = ATTRIB;
					}
				 	screen_x[alt_terminal] =0;
					screen_y[alt_terminal] =23;
				 }
			
		}
		else
		{
			 if(screen_x[alt_terminal]>=79)
			 {
				 screen_x[alt_terminal]=0;
				 screen_y[alt_terminal]++;
			 }
			 //put to alt video memeory
			*(uint8_t *)(VIDEO+(alt_terminal+1)*size_of_vmem + ((NUM_COLS*screen_y[alt_terminal] + screen_x[alt_terminal]) << 1)) = c;
			*(uint8_t *)(VIDEO+(alt_terminal+1)*size_of_vmem + ((NUM_COLS*screen_y[alt_terminal] + screen_x[alt_terminal]) << 1) + 1) = ATTRIB;
			 screen_x[alt_terminal] ++;
			 screen_x[alt_terminal] %= NUM_COLS;
				 if(screen_y[alt_terminal] >= 24)//scolling
				 {
					for(i=NUM_COLS; i< (NUM_ROWS*NUM_COLS-NUM_COLS); i++) {
					
						*(uint8_t *)(VIDEO+(alt_terminal+1)*size_of_vmem + (i << 1)) = *(uint8_t *)(VIDEO+(alt_terminal+1)*size_of_vmem + ((i+NUM_COLS)<< 1));
						*(uint8_t *)(VIDEO+(alt_terminal+1)*size_of_vmem + (i << 1) + 1) = *(uint8_t *)(VIDEO+(alt_terminal+1)*size_of_vmem + ((i+NUM_COLS)<< 1)+1);;
					}
					for(i=(NUM_ROWS*NUM_COLS-NUM_COLS); i< (NUM_ROWS*NUM_COLS); i++) {
						*(uint8_t *)(VIDEO+(alt_terminal+1)*size_of_vmem + (i << 1)) = ' ';
						*(uint8_t *)(VIDEO+(alt_terminal+1)*size_of_vmem + (i << 1) + 1) = ATTRIB;
					}
					 screen_x[alt_terminal] =0;
					 screen_y[alt_terminal] =23;
				 }
			}
		update_cursor(screen_y[alt_terminal], screen_x[alt_terminal]);

}


  /*putcur
  * Description: put a string to the alt_terminal
  * 	  Input: --buff: buffer stored the string
  *      Output: increment index
  * Return value: N/A
  * Side effect: N/A
  */
void
printscur( char * buff)
{
	register int32_t index = 0;
	while(buff[index] != '\0') {
		//if not reach the end, print the char. 
		putcur(buff[index]);
		// increment the index
		index++;
	}
}


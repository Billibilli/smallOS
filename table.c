#include "task.h"
#include "lib.h"
#include "table.h"

#define maxhist 20
/*********************************************************
 * this table is a data structure we referenced from internet
 * Description : It provides functions: push, pop and get elements 
 * 				 in table. Table functioned as a container like 
 * 				 queue.
 */
static char buffer[NUM_TERMINAL * maxhist ][1024];
static int buffer_cur[NUM_TERMINAL];
static int buffer_total[NUM_TERMINAL];
/* init_table
 * Description: initial the table 
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect: clear the buffer.
 */
void init_table(void)
{
	int i,j,k;
	for(i = 0; i < NUM_TERMINAL;i++)
	{
		for(j = 0; j < maxhist;j++)
			for(k = 0; k < 1024;k++)
		{
			buffer[i*maxhist + j ][k] = 0;
		}
		buffer_cur[i] = 0;
	}
}
/* uptable
 * Description: increase the table
 * 		 Input: -- buf: store the data from the terminal
 *				-- terminal: current terminal
 *       Output: N/A
 *		 Return value: the length stored
 * 		 Side effect: N/A
 */

int uptable(char * buf, uint32_t terminal)
{
	int k;
	// if(buffer_cur[terminal] < buffer_total[terminal])
	// {
		if((buffer_cur[terminal]) == 0)
		{
			return strlen(buf);
		}
		else
		{
			buffer_cur[terminal]--;		
			for(k = 0; k < 1024;k++)
				{
					buf[k]=buffer[terminal*maxhist + buffer_cur[terminal]][k];
				}
			return strlen(buffer[terminal*maxhist + buffer_cur[terminal]]);
		}
	
	// }
	// else
	// {
		// for(k = 0; k < strlen(buf);k++)
			// {
				// if((buf[k] != '\n') &&  (buf[k] != '\0' ))
				// buffer[terminal*maxhist + buffer_total[terminal]][k] = buf[k];
			// }
		// buffer_total[terminal]++;
		// buffer_cur[terminal] = buffer_total[terminal];
	// }
}


/* downtable
 * Description: decrease the table 
 * 		 Input: -- buf: store the data from the terminal
 *				-- terminal: current terminal
 *       Output: N/A
 *		 Return value: the length stored
 * 		 Side effect: N/A
 */

int downtable(char * buf, uint32_t terminal)
{
	int k;
	if((buffer_cur[terminal] ) ==  buffer_total[terminal])
	{
		return strlen(buf);
	}
	else
	{
		buffer_cur[terminal]++;
		for(k = 0; k < 1024;k++)
			{
				buf[k] = buffer[terminal*maxhist + buffer_cur[terminal]][k] ;
			}
		return strlen(buf);
	}
	
}

/* pushtable
 * Description: push the table 
 * 		 Input: -- buf: pop the data stored
 *				-- terminal: current terminal
 *       Output: N/A
 *		 Return value: the length stored
 * 		 Side effect: if the content reaches maximum, clear the oldest one. 
 */
void pushtable(char * buf, uint32_t terminal)
{	
	int j,k;
	if((buffer_total[terminal]) ==  maxhist)
	{
		for(j = 0; j <(maxhist - 1);j++)
				for(k = 0; k < 1024;k++)
			{
				buffer[terminal*maxhist + j ][k] = buffer[terminal*maxhist + j + 1][k];
			}
		for(k = 0; k < 1024;k++)
			{
				if((buf[k] != '\n') &&  (buf[k] != '\0' ))
				buffer[terminal*maxhist + j][k] = buf[k];
				else
				buffer[terminal*maxhist + j][k] = 0;
			}
		buffer_cur[terminal] = buffer_total[terminal];
	}
	else
	{	
	if((buf[0] == '\n') ||  (buf[0] == '\0' ))
	return;
		for(k = 0; k < 1024;k++)
			{
				if((buf[k] != '\n') &&  (buf[k] != '\0' ))
				buffer[terminal*maxhist + buffer_total[terminal]][k] = buf[k];
			}
		buffer_total[terminal]++;
		buffer_cur[terminal] = buffer_total[terminal];
	
	}
	

}



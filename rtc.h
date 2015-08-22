/* lib.h - Defines for useful library functions
 * vim:ts=4 noexpandtab
 */

#ifndef _RTC_H
#define _RTC_H
#include "lib.h" 

extern void open_rtc(void);
extern void set_rtc(void);
extern int write_rtc(int fre);
extern int read_rtc(void);
extern void happen(void);

#endif



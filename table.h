#ifndef _TABLE_H
#define _TABLE_H

extern int uptable(char * buf, uint32_t terminal);
extern int downtable(char * buf, uint32_t terminal);
extern void pushtable(char * buf, uint32_t terminal);
extern void init_table(void);
#endif













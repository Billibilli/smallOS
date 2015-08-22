#ifndef _TERMINAL_H
#define _TERMINAL_H
#include "types.h"
extern void read_terminal(int32_t fd, const void * buf, int32_t key);
extern void open_terminal(const uint8_t* filename);
extern void write_terminal(int32_t fd, void * buf, int32_t key);
extern int r_terminal(int32_t fd, uint8_t* buf, int32_t key);

#endif

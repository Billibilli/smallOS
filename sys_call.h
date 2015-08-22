#ifndef _SYS_CALL_H
#define _SYS_CALL_H
#include "types.h"

extern int32_t do_test (void);
extern int32_t do_halt (uint8_t status);
extern int32_t do_execute (const uint8_t* command);
extern int32_t do_read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t do_write (int32_t fd, const void* buf, int32_t nbytes);
extern int32_t do_open (const uint8_t* filename);
extern int32_t do_close (int32_t fd);
extern int32_t do_getargs (uint8_t* buf, int32_t nbytes);
extern int32_t do_vidmap (uint8_t** screen_start);
extern int32_t do_set_handler (int32_t signum, void* handler_address);
extern int32_t do_sigreturn (void);

#endif













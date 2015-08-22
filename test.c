#include "test.h"
#include "lib.h"

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "idt_handler.h"
#include "page.h"
#include "rtc.h"
#include "terminal.h"
#include "file_system.h"

int32_t 
test_system_call(int32_t arg1, int32_t *arg2, int32_t arg3, int32_t call_num)
{
  int res;
  __asm__ volatile(
    "int $0x80"        /* make the request to the OS */
    : "=a" (res) ,      /* return result in eax ("a") */
      "+b" (arg1),     /* pass arg1 in ebx ("b") */
      "+c" (arg2),     /* pass arg2 in ecx ("c") */
      "+d" (arg3)      /* pass arg3 in edx ("d") */
    : "a"  (call_num)       /* pass system call number in eax ("a") */
    : "memory", "cc"); /* announce to the compiler that the memory and condition codes have been modified */
 
  /* The operating system will return a negative value on error;
   * wrappers return -1 on error and set the errno global variable */

  return res;
}

/* test_dentries
 * Description: testing 
 * 		 Input: N/A
 *       Output: N/A
 *		 Return value: N/A
 * 		 Side effect:testing 
 */
void 
test_dentries()
{

	uint8_t buf[2000];
	int i;
	dentry_t dentry;
	uint8_t name[32] = "frame1.txt";
	read_dentry_by_name(name, &dentry);
	printf("name is %s  \n", dentry.name);
	printf("type is %x  \n", dentry.type);
	printf("index is %x  \n", dentry.inode);
	read_data( dentry.inode, 0x00, buf, 2000);
	for(i = 0; i <strlen((int8_t*)buf); i++ )
	{
	printf("%c", buf[i]);
	}

}










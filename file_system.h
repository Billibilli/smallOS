#include "types.h"
#ifndef _FILE_SYSTEM_H
#define _FILE_SYSTEM_H

#define FILE_BLOCK_SIZE 4096

typedef struct dentry{
	uint8_t name[32];
	uint32_t type;
	uint32_t inode;
	}dentry_t;
	
extern void setstart(uint32_t filestart); 
extern uint32_t sizeofinode(uint32_t inode);
extern int32_t get_inode_by_name(uint8_t* fname);
extern int32_t read_dentry_by_name(const uint8_t * fname, dentry_t *dentry );
extern int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry);
extern int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);
#endif 



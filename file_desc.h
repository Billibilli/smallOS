#ifndef _FILE_DESC_H
#define _FILE_DESC_H

typedef struct filedescriptor{
	void* file_operations_pointer;
	uint32_t inode_pointer;
	uint32_t file_position;
	uint32_t flags;
	}filedescriptor_t;


typedef struct file_type_op_action{
    int32_t (*open)(filedescriptor_t*,void*);
	int32_t (*read)(filedescriptor_t* , void*, uint32_t);
	int32_t (*write)(filedescriptor_t* , const void*, uint32_t);
	}file_type_op_action_t;
	

#endif 



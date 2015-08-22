#include "file_system.h"
#include "lib.h"
#include "types.h"
#define debug_file_system 0
#define debug_get_inode_by_name 0
#define debug_set_start 0
#define debug_read_dentry_by_name 0
#define debug_read_data 0
#define debug_check_name 0
static uint32_t filestart;
static int checkname(const uint8_t * fname, uint8_t * cname);

extern uint32_t sizeofinode(uint32_t inode)
{
return *(uint32_t*)(filestart + FILE_BLOCK_SIZE*(inode + 1));
};
/* 
 * get_inode_by_name
 *   Description: obtain the node 
 *	 INPUTS: fname -- file name which need to access
 *   OUTPUTS: N/A
 *   RETURN VALUE: return inode if succeed or -1 if failed
 *   SIDE EFFECTS: N/A 
 */
int32_t get_inode_by_name(uint8_t* fname){
	dentry_t copy;
#if debug_get_inode_by_name
printf("get_inode_by_name:center get_inode_by_name %s    ", fname);
#endif
	if(read_dentry_by_name((uint8_t *)fname,  &copy) == -1)
		return -1;
#if debug_get_inode_by_name		
printf("get_inode_by_name:copy.inode :%x:",copy.inode);
#endif
	return copy.inode;
}

/* 
 * setstart
 *   Description: set the starting addr
 *	 INPUTS: start -- starting addr
 *   OUTPUTS: N/A
 *   RETURN VALUE: N/A
 *   SIDE EFFECTS: N/A 
 */
void setstart(uint32_t start)
{
	filestart=start;
#if debug_set_start
printf("0x%x ", (uint32_t)*(uint32_t *)(filestart+4));
#endif
}

/* 
 * read_dentry_by_name
 *   Description: Search the file by name
 *	 INPUTS: dentry -- pointer points to the struct that we 
 *					   need to construct for the name comparing
 *					   info
 *			  fname --  the file name that need to be found
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: reconstruct the dentry every time failed to 
 *				   find the name. 
 */
int32_t read_dentry_by_name(const uint8_t * fname, dentry_t *dentry)
{
	/*Indicating the number of directory entries. */
	uint32_t numinde =  (uint32_t)*(uint32_t*)(filestart+4);
	int i;
	int end = 0;
	int h;
	uint32_t address;
	/*Search the entries one by one*/
	for( i = 1; i<=numinde; i++)
	{
		/*If we failed to find the file name, reconstruct the dentry */
		if(checkname(fname, (uint8_t*)(filestart + 64*i)))
		{
			for( h = 0; h<32; h++){
			/*construct the name buffer bit by bit for 32 times since it's 32bits*/
			dentry->name[h] = (uint8_t)*(uint8_t*) (filestart + 64*i + h);}
			/*the addr of type is 32bit after the name*/
			dentry->type = (uint32_t) *(uint32_t*)(filestart + 64*i + 32);
			/*the addr of inode is 4bit after the type*/
			dentry->inode = (uint32_t) *(uint32_t*)(filestart + 64*i + 36);
			/* calculate the address of the inode pointer*/
			address = (filestart + FILE_BLOCK_SIZE*(i+1));
			end = 1;
#if debug_read_dentry_by_name
			printf("read_dentry_by_name   '%s'  ",(uint8_t*)(filestart + 64*i)); 
			printf("read_dentry_by_name   '%s'  ",fname);
#endif
			break;
		}
#if debug_read_dentry_by_name
	       printf("read_dentry_by_name '%s'",(uint8_t*)(filestart + 64*i)); 
#endif
	}
	if(end == 1)
	{
#if debug_read_dentry_by_name
		printf("read_dentry_by_name: success on looking for '%s' ",fname); 
#endif
	return address;//return inode pointer
	}
	else
	{
	//	printf("failure on looking for'%s' ",fname); 
	return -1;
	}
	
};
/* 
 * checkname
 *   Description: check the name
 *	 INPUTS: fname -- file name need to access
 *			 cname -- expected name	
 *   OUTPUTS: N/A
 *   RETURN VALUE: return 0 if fail or 1 if succeed
 *   SIDE EFFECTS: N/A 
 */
int checkname(const uint8_t * fname, uint8_t * cname)
{
	int h;
	/*If the string length doesn't match, return 0*/
	if(strlen((int8_t*)cname) < strlen((int8_t*)fname))
		return 0;
		/*Compare the bits one by one*/	
	for( h = 0; h < strlen((int8_t*)cname); h++)
		if(fname[h] != cname[h])
		/*If not match, return 0*/
		return 0;

#if debug_check_name
	printf("checkname1 '%s'  ",fname);
	printf("checkname2 '%s'  ",cname); 
#endif
	return 1;

};
/* 
 * read_dentry_by_index
 *   Description: Search the file by index
 *	 INPUTS: dentry -- pointer points to the struct that we 
 *					   need to construct for the index comparing
 *					   info
 *			  index --  the file index that need to be found
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success, -1 on failure
 *   SIDE EFFECTS: none
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry)
{
	uint32_t numinde =  (uint32_t)*(uint32_t*)(filestart+4);
	int i;
	int end = 0;
	int h;
	for( i = 1; i<=numinde; i++)
	{
		/*We only need to identify the unsigned integer which is index*/
		if((uint32_t) *(uint32_t*)(filestart + 64*i + 36) == index)
		{
			/*If not find, do the same thing as read by name*/
			for( h = 0; h<32; h++)
			dentry->name[h] = (uint8_t) * (uint8_t*)(filestart + 64*i + h);
			dentry->type = (uint32_t) * (uint32_t*)(filestart + 64*i + 32);
			dentry->inode = (uint32_t) * (uint32_t*)(filestart + 64*i + 36);
			
			end = 1;
			break;
		}
	}
	if(end == 1)
	{
	//printf("can find in ");
	return 0;
	}
	else
	{//printf("cannot find in ");
	return -1;
	}

}
/* 
 * read_data
 *   Description: Search the file by name
 *	 INPUTS: inode -- the exact index node that we need access
 *			 offset -- the starting point start to access	   
 *			 buf -- pointer to the buffer which store the accessed data		   
 *			 length --  the length of the data need to copied
 *   OUTPUTS: none
 *   RETURN VALUE: the length of the data read or -1 on failure
 *   SIDE EFFECTS: none 
 */

int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length)
{
    /* Obtain the total number of inodes */
	uint32_t  total_inodes =(uint32_t ) *(uint32_t*) (filestart + 4);     
	/* get the addr of the inode of the file */
	uint32_t * dentry_ptr = (uint32_t*)(filestart + FILE_BLOCK_SIZE*(inode + 1)); 
	/* obtain offset in the block */																		  
	uint32_t charoffset = offset % FILE_BLOCK_SIZE ;   
	/* pointer to the data block */ 
	uint8_t * block_pointer;       
	/* data block index */                         
	uint32_t block_num;                                                  
	uint32_t i;
	if(inode > total_inodes)       /* offset > total # of bytes Or inode > total inodes */                                         // offset > total # of bytes Or inode > total inodes                                                       
	return -1;	
	
	if(inode != 0)
	{
		if(offset >= *dentry_ptr)
		return 0;
		for(i = 0; i < length; i++)      /* copy length blocks to the buffer in order */                                     
		{
		if(i + offset > *dentry_ptr)    /* if it reaches the end of the block, then break */
			break;
			block_num = *(dentry_ptr+1+(i+charoffset)/FILE_BLOCK_SIZE);  
			block_pointer = (uint8_t*) (filestart + (1+total_inodes + block_num) * FILE_BLOCK_SIZE);    
			buf[i] = (uint8_t) *(block_pointer+(i+charoffset)%FILE_BLOCK_SIZE); 		

		}
			return i;
	}
	else
	{
		strncpy((int8_t *)buf,((int8_t*) (filestart + 64*(offset / length + 1))),length);
		return strlen((int8_t*) (filestart + 64*(offset / length + 1)));
		
	}
#if debug_read_data
        printf(" read success%s",buf);
#endif


}




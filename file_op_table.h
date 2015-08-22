#ifndef _FILE_OP_TABLE_H
#define _FILE_OP_TABLE_H
#include "lib.h"
#include "idt_handler.h"
#include "rtc.h"
#include "file_system.h"
#include "terminal.h"
#include "file_desc.h"
#define debug_fread_f_d 0
#define debug_fread_f_d_freq 51
#if debug_fread_f_d
static int32_t debug_fread_f_d_k=debug_fread_f_d_freq;
#endif

static int32_t fopen_f_d(filedescriptor_t* fdstruct,void* buf)
{
	return -1;
}

static int32_t fread_f_d(filedescriptor_t* fdstruct, void* buf, uint32_t n_bytes)
{


	//char buff[n_bytes];
	//char * pointer =(char *) buf;
	//int32_t h;
	uint32_t temp;
	//uint32_t temp2;
	//uint32_t inode = fdstruct->inode_pointer;
	//uint32_t n_bytes1 = n_bytes;
	//printf("indoe %d",fdstruct->inode_pointer);
	//printf("fb %x",(uint32_t)*(uint32_t*)(fdstruct->inode_pointer));
	if(fdstruct->inode_pointer == 0)
	{
		//printf( "directory:  %d",fdstruct->file_position);
		temp = read_data(fdstruct->inode_pointer, fdstruct->file_position, (uint8_t*) buf, n_bytes);
		fdstruct->file_position += n_bytes;

	}
	else
	{
	temp = read_data(fdstruct->inode_pointer, fdstruct->file_position, (uint8_t*) buf, n_bytes);
		fdstruct->file_position += temp;
		if(temp == 0)
		{
			//printf( "cao ni ma quan jia ");

		}
		
	}
	// don grep excutebe file
	if(*(uint32_t*) buf == 0x464c457f )
	 return 0;
		//temp2 = fdstruct->file_position;
	// if(temp == -1)
		// fdstruct->file_position = 0;
		// for(h = temp; h < strlen((uint8_t*)buf); h++)
			// pointer[h]=0;
		//printf( "%c",*pointer);
#if debug_fread_f_d
	debug_fread_f_d_k--;
	if (debug_fread_f_d_k==0) {
	printf("Buf is %s\n",buf);
	printf("return data byte read:%d\n",temp);
	while(1);
	}
#endif
	//printf( "\n %s 	",buf);
	return temp; 

}

static int32_t fwrite_f_d(filedescriptor_t* fdstruct, const void* buf, uint32_t n_bytes)
{
	return -1;
}

static int32_t fopen_rtc(filedescriptor_t* fdstruct,void* buf)
{
return 0;
}

static int32_t fread_rtc(filedescriptor_t* fdstruct, void* buf, uint32_t n_bytes)
{
sti();
read_rtc();
cli();
return 0;
}

static int32_t fwrite_rtc(filedescriptor_t* fdstruct, const void* buf, uint32_t n_bytes)
{

	// printf( "fwrite_rtc %d",(int)*(int*)buf);
	// char *type = NULL;
	// char temp  =*type;
	cli();
	write_rtc((int)*(int*)buf);
	sti();
return 0;
}

static int32_t termin_read(filedescriptor_t* fdstruct, void* buf, uint32_t n_bytes)
{
sti();
return r_terminal(0x00,(uint8_t*) buf, n_bytes);


}

static int32_t termin_open(filedescriptor_t* fdstruct,void* buf)
{
open_terminal(0x00);
return 0;
}

static int32_t termin_write(filedescriptor_t* fdstruct, const void* buf, uint32_t n_bytes)
{

printf( "%s",buf);
return 0;
}


//no
file_type_op_action_t RTC_type_op = {
.open =&fopen_rtc, 
.write= &fwrite_rtc, 
.read= &fread_rtc};



//DIRECTORY 
file_type_op_action_t directry_type_op = {
.open = &fopen_f_d, 
.write= &fwrite_f_d, 
.read = &fread_f_d};



//REGULER FILE
file_type_op_action_t file_type_op = {
.open = &fopen_f_d, 
.write= &fwrite_f_d, 
.read = &fread_f_d};

file_type_op_action_t terminal_type_op = {
.open = &termin_open, 
.write= &termin_write, 
.read = &termin_read};


#endif 



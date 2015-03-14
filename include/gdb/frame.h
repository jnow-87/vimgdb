#ifndef GDB_FRAME_H
#define GDB_FRAME_H


#include <gdb/result.h>


class gdb_frame_t{
public:
	gdb_frame_t();
	~gdb_frame_t();

	void* addr;
	unsigned int line;
	
	char *function,
		 *filename,
		 *fullname;
};


int conv_frame(gdb_result_t* result, gdb_frame_t** frame);


#endif // GDB_FRAME_H

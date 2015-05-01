#ifndef GDB_FRAME_H
#define GDB_FRAME_H


#include <gdb/result.h>
#include <gdb/variable.h>


class gdb_frame_t{
public:
	static int result_to_frame(gdb_result_t* result, gdb_frame_t** frame);

	gdb_frame_t();
	~gdb_frame_t();

	void* addr;
	unsigned int line,
				 level;
	
	char *function,
		 *filename,
		 *fullname;

	bool expanded;

	gdb_variable_t *args,
				   *locals;

	gdb_frame_t *next,
				*prev;
};


#endif // GDB_FRAME_H

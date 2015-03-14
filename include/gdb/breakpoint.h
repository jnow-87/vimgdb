#ifndef BREAKPOINT_H
#define BREAKPOINT_H


#include <gdb/result.h>


/* types */
class gdb_breakpoint_t{
public:
	gdb_breakpoint_t();
	~gdb_breakpoint_t();


	unsigned int num;

	unsigned int line;
	char *filename,
		 *fullname,
		 *at;

	bool enabled;
};


/* prototypes */
int conv_break_insert(gdb_result_t* result, void** bkpt);
int conv_breakpoint(gdb_result_t* result, gdb_breakpoint_t** bkpt);


#endif // BREAKPOINT_H

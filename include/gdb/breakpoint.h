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
int result_to_brkpt(gdb_result_t* result, void** bkpt);


#endif // BREAKPOINT_H

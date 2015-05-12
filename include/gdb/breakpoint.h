#ifndef BREAKPOINT_H
#define BREAKPOINT_H


#include <gdb/result.h>


class gdb_breakpoint_t{
public:
	static int result_to_brkpt(gdb_result_t* result, void** bkpt);

	gdb_breakpoint_t();
	~gdb_breakpoint_t();


	unsigned int num;
	bool temporary;

	unsigned int line;
	char *filename,
		 *fullname,
		 *at,
		 *condition,
		 *ignore_cnt;

	bool enabled;
};


#endif // BREAKPOINT_H

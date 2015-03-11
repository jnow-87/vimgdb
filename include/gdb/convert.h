#ifndef CONVERT_H
#define CONVERT_H


#include <gdb/result.h>


struct gdb_convert_t{
	const char* name;
	int (*cb)(gdb_result_t*, void**);
};

typedef struct gdb_convert_t gdb_convert_t;


#endif // CONVERT_H

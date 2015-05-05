#ifndef GDB_MEMORY_H
#define GDB_MEMORY_H


#include <gdb/result.h>


class gdb_memory_t{
public:
	static int result_to_memory(gdb_result_t* result, void** mem);


	gdb_memory_t();
	~gdb_memory_t();


	char *begin,
		 *content;

	unsigned int length;

	gdb_memory_t *next,
			     *prev;
};


#endif // GDB_MEMORY_H

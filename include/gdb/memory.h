#ifndef GDB_MEMORY_H
#define GDB_MEMORY_H


#include <gdb/result.h>


class gdb_memory_t : public gdb_result_t{
public:
	gdb_memory_t();
	~gdb_memory_t();


	char *begin,
		 *content;

	unsigned int length;
	bool expanded;

	gdb_memory_t *next,
			     *prev;
};


#endif // GDB_MEMORY_H

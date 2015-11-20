#ifndef GDB_MEMORY_H
#define GDB_MEMORY_H


#include <gdb/result.h>


class gdb_memory_t : public gdb_result_t{
public:
	static gdb_memory_t* acquire();
	static gdb_memory_t* acquire(void* addr, unsigned int length);
	static gdb_memory_t* acquire(char* addr, unsigned int length);
	static int set(void* addr, char* value, unsigned int cnt = 0);

	~gdb_memory_t();

	int update();

	char *begin,
		 *content,
		 *content_old;

	unsigned int length,
				 alignment;
	bool expanded;

	gdb_memory_t *next,
			     *prev;

private:
	gdb_memory_t();
};


#endif // GDB_MEMORY_H

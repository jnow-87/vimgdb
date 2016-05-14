#ifndef GDB_STRING_H
#define GDB_STRING_H


#include <gdb/result.h>


class gdb_strlist_t : public gdb_result_t{
public:
	gdb_strlist_t();
	gdb_strlist_t(char *s);
	~gdb_strlist_t();

	char *s;

	class gdb_strlist_t *next,
					    *prev;
};


#endif // GDB_STRING_H

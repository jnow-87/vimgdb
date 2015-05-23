#ifndef GDB_STRING_H
#define GDB_STRING_H


class gdb_strlist_t{
public:
	gdb_strlist_t();
	gdb_strlist_t(char* s);

	char* s;

	class gdb_strlist_t *next,
						   *prev;
};


#endif // GDB_STRING_H

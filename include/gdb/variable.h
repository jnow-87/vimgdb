#ifndef GDB_VARIABLE_H
#define GDB_VARIABLE_H


#include <gdb/result.h>


/* types */
class gdb_variable_t{
public:
	gdb_variable_t();
	~gdb_variable_t();

	char *name,
		 *exp,
		 *type,
		 *value;

	bool modified,
		 childs_visible;

	unsigned int nchilds;

	class gdb_variable_t *next,
						 *prev,
						 *parent,
						 *childs;
};


/* prototypes */
int result_to_variable(gdb_result_t* result, void** var);
int result_to_change_list(gdb_result_t* result, void** var_lst);


#endif // GDB_VARIABLE_H

#ifndef GDB_VARIABLE_H
#define GDB_VARIABLE_H


#include <gdb/result.h>
#include <gdb/gdb.h>


/* types */
typedef enum{
	O_UNKNOWN = 0,
	O_STACK,
	O_USER,
} gdb_varorigin_t;

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

	gdb_varorigin_t origin;

	class gdb_variable_t *next,
						 *prev,
						 *parent,
						 *childs;
};


/* external variables */
extern map<string, gdb_variable_t*> gdb_var_lst;


/* prototypes */
int gdb_variables_update();

int result_to_variable(gdb_result_t* result, void** var);
int result_to_change_list(gdb_result_t* result, void** unused);


#endif // GDB_VARIABLE_H

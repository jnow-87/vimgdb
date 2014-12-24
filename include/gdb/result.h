#ifndef GDB_RESULT_H
#define GDB_RESULT_H


#include <gdb/value.h>


/* types */
typedef struct _result_t{
	char* var_name;
	value_t* value;

	struct _result_t *next, *prev;
} result_t;


/* prototypes */
result_t* gdb_result_create(char* var_name, value_t* value);
result_t* gdb_result_free(result_t* list);
void gdb_result_add(result_t* list, result_t* result);
void gdb_result_print(result_t* list);


#endif // GDB_RESULT_H

#ifndef GDB_RESULT_H
#define GDB_RESULT_H


#include <gdb/value.h>


/* types */
typedef enum{
	RC_DONE = 1,
	RC_RUNNING,
	RC_CONNECTED,
	RC_ERROR,
	RC_EXIT,
} result_class_t;

typedef enum{
	AC_STOPPED = 1,
	AC_BREAK_CREATED,
	AC_THREAD_GRP_ADDED,
} async_class_t;

typedef enum{
	SC_CONSOLE = 1,
	SC_TARGET,
	SC_LOG,
} stream_class_t;

typedef enum{
	V_ID = 1,
	V_MSG,
} variable_id_t;

typedef struct _result_t{
	char* var_name;
	variable_id_t var_id;

	value_t* value;

	struct _result_t *next, *prev;
} result_t;


/* prototypes */
result_t* gdb_result_create(char* var_name, variable_id_t var_id, value_t* value);
result_t* gdb_result_free(result_t* list);
void gdb_result_add(result_t* list, result_t* result);
void gdb_result_print(result_t* list);


#endif // GDB_RESULT_H

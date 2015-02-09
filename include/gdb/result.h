#ifndef GDB_RESULT_H
#define GDB_RESULT_H


#include <gdb/value.h>
#include <gdb/variable.h>


/* types */
typedef enum{
	RC_DONE = 1,
	RC_RUNNING,
	RC_CONNECTED,
	RC_ERROR,
	RC_EXIT,
	RC_STOPPED,
	RC_BREAK_CREATED,
	RC_THREAD_CREATED,
	RC_THREAD_GRP_ADDED,
	RC_THREAD_GRP_STARTED,
	RC_PARAM_CHANGED,
} result_class_t;

typedef enum{
	SC_CONSOLE = 1,
	SC_TARGET,
	SC_LOG,
} stream_class_t;

typedef struct _result_t{
	const char* var_name;
	variable_id_t var_id;

	value_t* value;

	struct _result_t *next, *prev;
} result_t;


/* prototypes */
result_t* gdb_result_create(const char* var_name, variable_id_t var_id, value_t* value);
result_t* gdb_result_free(result_t* list);
void gdb_result_add(result_t* list, result_t* result);
void gdb_result_print(result_t* list);


#endif // GDB_RESULT_H

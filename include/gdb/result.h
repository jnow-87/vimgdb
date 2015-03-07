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
} gdb_result_class_t;

typedef enum{
	SC_CONSOLE = 1,
	SC_TARGET,
	SC_LOG,
} gdb_stream_class_t;

typedef struct _gdb_result_t{
	const char* var_name;
	gdb_var_id_t var_id;

	gdb_value_t* value;

	struct _gdb_result_t *next, *prev;
} gdb_result_t;


/* prototypes */
gdb_result_t* gdb_result_create(const char* var_name, gdb_var_id_t var_id, gdb_value_t* value);
gdb_result_t* gdb_result_free(gdb_result_t* list);
void gdb_result_add(gdb_result_t* list, gdb_result_t* result);
void gdb_result_print(gdb_result_t* list);


#endif // GDB_RESULT_H

#ifndef GDB_RESULT_H
#define GDB_RESULT_H


#include <gdb/value.h>
#include <gdb/variable.h>


/* types */
typedef enum{
	RC_DONE = 0x1,
	RC_RUNNING = 0x2,
	RC_CONNECTED = 0x4,
	RC_ERROR = 0x8,
	RC_EXIT = 0x10,
	RC_STOPPED = 0x20,
	RC_BREAK_CREATED = 0x40,
	RC_THREAD_CREATED = 0x80,
	RC_THREAD_GRP_ADDED = 0x100,
	RC_THREAD_GRP_STARTED = 0x200,
	RC_PARAM_CHANGED = 0x400,
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

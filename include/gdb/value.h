#ifndef GDB_VALUE_H
#define GDB_VALUE_H


/* types */
typedef enum{
	VT_EMPTY = 0,
	VT_CONST,			// value is of type char*
	VT_VALUE_LIST,		// value is of type gdb_value_t*
	VT_RESULT_LIST,	// value is of type result_t*
}  gdb_value_type_t;

typedef struct _value_t{
	 gdb_value_type_t type;
	void* value;

	struct _value_t *next, *prev;
} gdb_value_t;


/* prototypes */
gdb_value_t* gdb_value_create( gdb_value_type_t type, void* value);
gdb_value_t* gdb_value_free(gdb_value_t* value);
void gdb_value_add(gdb_value_t* list, gdb_value_t* value);
void gdb_value_print(gdb_value_t* list);


#endif // GDB_VALUE_H

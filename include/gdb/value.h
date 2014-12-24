#ifndef GDB_VALUE_H
#define GDB_VALUE_H


/* types */
typedef enum{
	EMPTY = 0,
	CONST,			// value is of type char*
	VALUE_LIST,		// value is of type value_t*
	RESULT_LIST,	// value is of type result_t*
} value_type_t;

typedef struct _value_t{
	value_type_t type;
	void* value;

	struct _value_t *next, *prev;
} value_t;


/* prototypes */
value_t* gdb_value_create(value_type_t type, void* value);
value_t* gdb_value_free(value_t* value);
void gdb_value_add(value_t* list, value_t* value);
void gdb_value_print(value_t* list);


#endif // GDB_VALUE_H

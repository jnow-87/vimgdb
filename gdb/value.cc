#include <common/list.h>
#include <common/log.h>
#include <gdb/value.h>
#include <gdb/result.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/* external variables */
extern unsigned int rec_depth;


/* global functions */
gdb_value_t* gdb_value_create( gdb_value_type_t type, void* value){
	gdb_value_t* v;


	v = (gdb_value_t*)malloc(sizeof(gdb_value_t));
	if(v == 0)
		return 0;

	// in case type == CONST, value is assumed to be allocated
	// properly, i.e. the string will stay available
	// in case of the lexer this is true
	v->value = value;
	v->type = type;
	list_init(v);

	return v;
}

gdb_value_t* gdb_value_free(gdb_value_t* value){
	gdb_value_t* v;


	list_for_each(value, v){
		switch(v->type){
		case VT_CONST:
			free(v->value);	// free the string allocated in lexer
			break;

		case VT_VALUE_LIST:
			v->value = gdb_value_free((gdb_value_t*)v->value);
			break;

		case VT_RESULT_LIST:
			v->value = gdb_result_free((gdb_result_t*)v->value);
			break;

		case VT_EMPTY:
			break;
		}

		list_rm(&value, v);
		free(v);
	}

	return value;
}

void gdb_value_add(gdb_value_t* list, gdb_value_t* value){
	list_add_tail(&list, value);
}

void gdb_value_print(gdb_value_t* list){
	unsigned int i;
	char rec_str[10];
	gdb_value_t* v;


	snprintf(rec_str, 10, "%%%ds", rec_depth);
	
	USER("list %#x{\n\n", list);
	i = 0;
	list_for_each(list, v){
		switch(v->type){
		case VT_CONST:
			USER(rec_str, "");
			USER("   %d: %s\n", i, v->value);
			break;

		case VT_VALUE_LIST:
			USER(rec_str, "");
			USER("   %d: ", i);

			rec_depth += 3;
			gdb_value_print((gdb_value_t*)v->value);
			rec_depth -= 3;
			break;
		
		case VT_RESULT_LIST:
			USER(rec_str, "");
			USER("   %d: ", i);

			rec_depth += 3;
			gdb_result_print((gdb_result_t*)v->value);
			rec_depth -= 3;
			break;

		case VT_EMPTY:
			break;
		};

		i++;
	}

	USER("\n");
	USER(rec_str, "");
	USER("} // list %#x\n\n", list);
}

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
value_t* gdb_value_create(value_type_t type, void* value){
	value_t* v;


	DEBUG("create value (%d, %#x)\n", type, value);
	v = (value_t*)malloc(sizeof(value_t));
	if(v == 0)
		goto err_0;

	// in case type == CONST, value is assumed to be allocated
	// properly, i.e. the string will stay available
	// in case of the lexer this is true
	v->value = value;
	v->type = type;
	list_init(v);

	DEBUG("end\n\n");

	return v;

err_1:
	free(v);

err_0:
	DEBUG("end (error)\n\n");
	return 0;
}

value_t* gdb_value_free(value_t* value){
	value_t* v;


	list_for_each(value, v){
		DEBUG("free value (%#x)\n", value);

		switch(v->type){
		case CONST:
			DEBUG("v->value is string \"%s\"\n", v->value);
			free(v->value);	// free the string allocated in lexer
			break;

		case VALUE_LIST:
			v->value = gdb_value_free((value_t*)v->value);
			break;

		case RESULT_LIST:
			v->value = gdb_result_free((result_t*)v->value);
			break;

		case EMPTY:
			break;
		}

		DEBUG("free value mem\n");

		list_rm(&value, v);
		free(v);

		DEBUG("end\n\n");
	}

	return value;
}

void gdb_value_add(value_t* list, value_t* value){
	DEBUG("add value %#x to %#x\n\n", value, list);
	list_add_tail(list, value);
}

void gdb_value_print(value_t* list){
	unsigned int i;
	char rec_str[10];
	value_t* v;


	snprintf(rec_str, 10, "%%%ds", rec_depth);
	
	DEBUG("list %#x{\n\n", list);
	i = 0;
	list_for_each(list, v){
		switch(v->type){
		case CONST:
			DEBUG(rec_str, "");
			DEBUG("   %d: %s\n", i, v->value);
			break;

		case VALUE_LIST:
			DEBUG(rec_str, "");
			DEBUG("   %d: ", i);

			rec_depth += 3;
			gdb_value_print((value_t*)v->value);
			rec_depth -= 3;
			break;
		
		case RESULT_LIST:
			DEBUG(rec_str, "");
			DEBUG("   %d: ", i);

			rec_depth += 3;
			gdb_result_print((result_t*)v->value);
			rec_depth -= 3;
			break;

		case EMPTY:
			break;
		};

		i++;
	}

	DEBUG("\n");
	DEBUG(rec_str, "");
	DEBUG("} // list %#x\n\n", list);
}

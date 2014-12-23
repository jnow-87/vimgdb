#include <stdio.h>
#include <string.h>
#include "list.h"
#include "xmalloc.h"
#include "gdb_value.h"
#include "gdb_result.h"


/* external variables */
extern unsigned int token;
extern unsigned int rec_depth;


/* global functions */
value_t* gdb_value_create(value_type_t type, void* value){
	value_t* v;
	unsigned int tk = token;
	token++;

printf("[%d] create value (%d, %#x)\n", tk, type, value);
	v = xmalloc(sizeof(value_t));
	if(v == 0)
		goto err_0;

	// in case type == CONST, value is assumed to be allocated
	// properly, i.e. the string will stay available
	// in case of the lexer this is true
	v->value = value;
	v->type = type;
	list_init(v);

printf("[%d] end\n\n", tk);

	return v;

err_1:
	xfree(v);

err_0:
printf("[%d] end (error)\n\n", tk);
	return 0;
}

value_t* gdb_value_free(value_t* value){
	value_t* v;
	unsigned int tk;


	list_for_each(value, v){
		tk = token;
		token++;

printf("[%d] free value (%#x)\n", tk, value);
		switch(v->type){
		case CONST:
printf("[%d] v->value is string \"%s\"\n", tk, v->value);
			xfree(v->value);	// free the string allocated in lexer
			break;

		case VALUE_LIST:
			v->value = gdb_value_free(v->value);
			break;

		case RESULT_LIST:
			v->value = gdb_result_free(v->value);
			break;

		case EMPTY:
			break;
		}

printf("[%d] free value mem\n", tk);

		list_rm(&value, v);
		xfree(v);

printf("[%d] end\n\n", tk);
	}

	return value;
}

void gdb_value_add(value_t* list, value_t* value){
printf("[%d] add value %#x to %#x\n\n", token++, value, list);

	list_add_tail(list, value);
}

void gdb_value_print(value_t* list){
	unsigned int i;
	char rec_str[10];
	value_t* v;


	snprintf(rec_str, 10, "%%%ds", rec_depth);
	
	printf("list %#x{\n\n", list);
	i = 0;
	list_for_each(list, v){
		switch(v->type){
		case CONST:
			printf(rec_str, "");
			printf("   %d: %s\n", i, v->value);
			break;

		case VALUE_LIST:
			printf(rec_str, "");
			printf("   %d: ", i);

			rec_depth += 3;
			gdb_value_print(v->value);
			rec_depth -= 3;
			break;
		
		case RESULT_LIST:
			printf(rec_str, "");
			printf("   %d: ", i);

			rec_depth += 3;
			gdb_result_print(v->value);
			rec_depth -= 3;
			break;

		case EMPTY:
			break;
		};

		i++;
	}

	printf("\n");
	printf(rec_str, "");
	printf("} // list %#x\n\n", list);
}

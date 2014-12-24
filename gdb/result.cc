#include <common/list.h>
#include <common/xmalloc.h>
#include <gdb/result.h>
#include <stdio.h>
#include <string.h>


/* external variables */
extern unsigned int token;

/* local variables */
unsigned int rec_depth = 0;


/* global functions */
result_t* gdb_result_create(char* var_name, value_t* value){
	result_t* r;
	unsigned int tk = token;
	token++;


printf("[%d] create result (\"%s\", %#x)\n", tk, var_name, value);

	r = (result_t*)xmalloc(sizeof(result_t));
	if(r == 0)
		goto err_0;

	// var_name is assumed to be properly allocated, i.e. the
	// string will stay available
	// in case of the lexer this is true
	r->var_name = var_name;
	r->value = value;
	list_init(r);

printf("[%d] end\n\n", tk);

	return r;

err_1:
	xfree(r);

err_0:

printf("[%d] end (error)\n\n", tk);
	return 0;
}

result_t* gdb_result_free(result_t* list){
	result_t* r;
	unsigned int tk;


	list_for_each(list, r){
		tk = token;
		token++;
printf("[%d] free result (\"%s\", %#x)\n", tk, r->var_name, r);

		list_rm(&list, r);
		r->value = gdb_value_free((value_t*)r->value);

printf("[%d] free result mem\n", tk);
		xfree(r->var_name);	// string allocated in lexer
		xfree(r);
printf("[%d] end\n\n", tk);
	}

	return list;
}

void gdb_result_add(result_t* list, result_t* result){
printf("[%d] add result %#x to %#x\n\n", token++, result, list);
	list_add_tail(list, result);
}

void gdb_result_print(result_t* list){
	char rec_str[10];
	result_t* r;


	snprintf(rec_str, 10, "%%%ds", rec_depth);
	
	list_for_each(list, r){
		printf(rec_str, "");
		printf("result %s = ", r->var_name);
		gdb_value_print((value_t*)r->value);
	}
}

#include <common/xmalloc.h>
#include <common/list.h>
#include <common/log.h>
#include <gdb/result.h>
#include <stdio.h>
#include <string.h>


/* local variables */
unsigned int rec_depth = 0;


/* global functions */
result_t* gdb_result_create(char* var_name, value_t* value){
	result_t* r;


	DEBUG("create result (\"%s\", %#x)\n", var_name, value);

	r = (result_t*)xmalloc(sizeof(result_t));
	if(r == 0)
		goto err_0;

	// var_name is assumed to be properly allocated, i.e. the
	// string will stay available
	// in case of the lexer this is true
	r->var_name = var_name;
	r->value = value;
	list_init(r);

	DEBUG("end\n\n");

	return r;

err_1:
	xfree(r);

err_0:

	DEBUG("end (error)\n\n");
	return 0;
}

result_t* gdb_result_free(result_t* list){
	result_t* r;


	list_for_each(list, r){
		DEBUG("free result (\"%s\", %#x)\n", r->var_name, r);

		list_rm(&list, r);
		r->value = gdb_value_free((value_t*)r->value);

		DEBUG("free result mem\n");
		xfree(r->var_name);	// string allocated in lexer
		xfree(r);
		DEBUG("end\n\n");
	}

	return list;
}

void gdb_result_add(result_t* list, result_t* result){
	DEBUG("add result %#x to %#x\n\n", result, list);
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

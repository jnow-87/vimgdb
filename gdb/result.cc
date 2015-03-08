#include <common/list.h>
#include <common/log.h>
#include <gdb/result.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/* local variables */
unsigned int rec_depth = 0;


/* global functions */
gdb_result_t* gdb_result_create(const char* var_name, gdb_var_id_t var_id, gdb_value_t* value){
	gdb_result_t* r;


	r = (gdb_result_t*)malloc(sizeof(gdb_result_t));
	if(r == 0)
		goto err_0;

	// var_name is assumed to be properly allocated, i.e. the
	// string will stay available
	// in case of the lexer this is true
	r->var_name = var_name;
	r->var_id = var_id;
	r->value = value;
	list_init(r);

	return r;

err_1:
	free(r);

err_0:
	return 0;
}

gdb_result_t* gdb_result_free(gdb_result_t* list){
	gdb_result_t* r;


	list_for_each(list, r){
		list_rm(&list, r);
		r->value = gdb_value_free((gdb_value_t*)r->value);
		free(r);
	}

	return list;
}

void gdb_result_add(gdb_result_t* list, gdb_result_t* result){
	list_add_tail(&list, result);
}

void gdb_result_print(gdb_result_t* list){
	char rec_str[10];
	gdb_result_t* r;


	snprintf(rec_str, 10, "%%%ds", rec_depth);
	
	list_for_each(list, r){
		USER(rec_str, "");
		USER("result %s (%d) = ", r->var_name, r->var_id);
		gdb_value_print((gdb_value_t*)r->value);
	}
}

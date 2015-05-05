#include <common/list.h>
#include <gdb/frame.h>
#include <string.h>
#include <stdlib.h>


gdb_frame_t::gdb_frame_t(){
	function = 0;
	filename = 0;
	fullname = 0;
	args = 0;
	locals = 0;
	expanded = false;
	next = 0;
	prev = 0;
}

gdb_frame_t::~gdb_frame_t(){
	gdb_variable_t* v;


	delete function;
	delete filename;
	delete fullname;

	list_for_each(args, v){
		list_rm(&args, v);
		gdb_variable_t::release(v);
	}

	list_for_each(locals, v){
		list_rm(&locals, v);
		gdb_variable_t::release(v);
	}
}

int gdb_frame_t::result_to_frame(gdb_result_t* result, gdb_frame_t** frame){
	gdb_result_t* r;


	if(*frame == 0)
		*frame = new gdb_frame_t;

	list_for_each(result, r){
		switch(r->var_id){
		case IDV_ADDRESS:
			(*frame)->addr = (void*)strtoll((char*)r->value->value, 0, 16);
			break;

		case IDV_LINE:
			(*frame)->line = atoi((char*)r->value->value);
			break;

		case IDV_LEVEL:
			(*frame)->level = atoi((char*)r->value->value);
			break;

		case IDV_FUNCTION:
			delete (*frame)->function;
			(*frame)->function = (char*)r->value->value;
			r->value->value = 0;
			break;

		case IDV_FILE:
			delete (*frame)->filename;
			(*frame)->filename = (char*)r->value->value;
			r->value->value = 0;
			break;

		case IDV_FULLNAME:
			delete (*frame)->fullname;
			(*frame)->fullname = (char*)r->value->value;
			r->value->value = 0;
			break;
		};
	}
	
	return 0;
}

#include <common/list.h>
#include <gdb/frame.h>
#include <string.h>
#include <stdlib.h>


gdb_frame_t::gdb_frame_t(){
	function = 0;
	filename = 0;
	fullname = 0;
}

gdb_frame_t::~gdb_frame_t(){
	delete function;
	delete filename;
	delete fullname;
}

int gdb_frame_t::result_to_frame(gdb_result_t* result, gdb_frame_t** frame){
	gdb_result_t* r;


	if(*frame == 0)
		*frame = new gdb_frame_t;

	list_for_each(result, r){
		switch(r->var_id){
		case IDV_ADDRESS:
			(*frame)->addr = (void*)atol((char*)r->value->value);
			break;

		case IDV_LINE:
			(*frame)->line = atoi((char*)r->value->value);
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

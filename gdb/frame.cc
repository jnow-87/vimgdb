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

int conv_frame(gdb_result_t* result, gdb_frame_t** frame){
	gdb_result_t* r;


	if(*frame == 0)
		*frame = new gdb_frame_t;

	list_for_each(result, r){
		switch(r->var_id){
		case V_ADDRESS:
			(*frame)->addr = (void*)atol((char*)r->value->value);
			break;

		case V_LINE:
			(*frame)->line = atoi((char*)r->value->value);

		case V_FUNCTION:
			(*frame)->function = new char[strlen((const char*)r->value->value) + 1];
			strcpy((*frame)->function, (const char*)r->value->value);
			break;

		case V_FILE:
			(*frame)->filename = new char[strlen((const char*)r->value->value) + 1];
			strcpy((*frame)->filename, (const char*)r->value->value);
			break;

		case V_FULLNAME:
			(*frame)->fullname = new char[strlen((const char*)r->value->value) + 1];
			strcpy((*frame)->fullname, (const char*)r->value->value);
			break;
		};
	}
	
	return 0;
}

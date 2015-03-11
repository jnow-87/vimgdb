#include <common/list.h>
#include <gdb/breakpoint.h>
#include <stdlib.h>
#include <string.h>
#include <common/log.h>


gdb_breakpoint_t::gdb_breakpoint_t(){
	filename = 0;
	fullname = 0;
	at = 0;
}

gdb_breakpoint_t::~gdb_breakpoint_t(){
	delete filename;
	delete fullname;
	delete at;
}


int conv_breakpoint(gdb_result_t* result, void** bkpt){
	gdb_result_t* r;


	if(*bkpt == 0)
		*bkpt = new gdb_breakpoint_t;

	list_for_each(result, r){
		switch(r->var_id){
		case V_NUMBER:
			((gdb_breakpoint_t*)*bkpt)->num = atoi((char*)r->value->value);
			break;

		case V_LINE:
			((gdb_breakpoint_t*)*bkpt)->line = atoi((char*)r->value->value);
			break;

		case V_FILE:
			((gdb_breakpoint_t*)*bkpt)->filename = new char[strlen((const char*)r->value->value) + 1];
			strcpy(((gdb_breakpoint_t*)*bkpt)->filename, (const char*)r->value->value);
			break;

		case V_FULLNAME:
			((gdb_breakpoint_t*)*bkpt)->fullname = new char[strlen((const char*)r->value->value) + 1];
			strcpy(((gdb_breakpoint_t*)*bkpt)->fullname, (const char*)r->value->value);
			break;

		case V_ENABLED:
			((gdb_breakpoint_t*)*bkpt)->enabled = (strcmp((const char*)r->value->value, "y") == 0) ? true : false;
			break;

		case V_AT:
			((gdb_breakpoint_t*)*bkpt)->at = new char[strlen((const char*)r->value->value) + 1];
			strcpy(((gdb_breakpoint_t*)*bkpt)->at, (const char*)r->value->value);
			break;

		default:
			if(r->value->type == VT_RESULT_LIST)
				conv_breakpoint((gdb_result_t*)r->value->value, bkpt);
		};
	}

	return 0;
}

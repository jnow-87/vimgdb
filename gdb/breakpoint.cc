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

int conv_break_insert(gdb_result_t* result, void** bkpt){
	gdb_result_t* r;


	list_for_each(result, r){
		switch(r->var_id){
		case V_BREAKPT:
			return conv_breakpoint((gdb_result_t*)r->value->value, (gdb_breakpoint_t**)bkpt);
			break;

		default:
			return -1;
		};
	}
}

int conv_breakpoint(gdb_result_t* result, gdb_breakpoint_t** bkpt){
	gdb_result_t* r;


	if(*bkpt == 0)
		*bkpt = new gdb_breakpoint_t;

	list_for_each(result, r){
		switch(r->var_id){
		case V_NUMBER:
			(*bkpt)->num = atoi((char*)r->value->value);
			break;

		case V_LINE:
			(*bkpt)->line = atoi((char*)r->value->value);
			break;

		case V_FILE:
			(*bkpt)->filename = new char[strlen((const char*)r->value->value) + 1];
			strcpy((*bkpt)->filename, (const char*)r->value->value);
			break;

		case V_FULLNAME:
			(*bkpt)->fullname = new char[strlen((const char*)r->value->value) + 1];
			strcpy((*bkpt)->fullname, (const char*)r->value->value);
			break;

		case V_ENABLED:
			(*bkpt)->enabled = (strcmp((const char*)r->value->value, "y") == 0) ? true : false;
			break;

		case V_AT:
			(*bkpt)->at = new char[strlen((const char*)r->value->value) + 1];
			strcpy((*bkpt)->at, (const char*)r->value->value);
			break;
		};
	}

	return 0;
}

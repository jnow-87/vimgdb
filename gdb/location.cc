#include <common/list.h>
#include <gdb/location.h>
#include <stdlib.h>
#include <string.h>
#include <common/log.h>


gdb_location_t::gdb_location_t(){
	fullname = 0;
	filename = 0;
}

gdb_location_t::~gdb_location_t(){
	delete fullname;
	delete filename;
}


int result_to_location(gdb_result_t* result, void** loc){
	gdb_result_t* r;


	if(*loc == 0)
		*loc = new gdb_location_t;

	list_for_each(result, r){
		switch(r->var_id){
		case IDV_LINE:
			((gdb_location_t*)*loc)->line = atoi((char*)r->value->value);
			break;

		case IDV_FILE:
			((gdb_location_t*)*loc)->filename = new char[strlen((const char*)r->value->value) + 1];
			strcpy(((gdb_location_t*)*loc)->filename, (const char*)r->value->value);
			break;

		case IDV_FULLNAME:
			((gdb_location_t*)*loc)->fullname = new char[strlen((const char*)r->value->value) + 1];
			strcpy(((gdb_location_t*)*loc)->fullname, (const char*)r->value->value);
			break;

		default:
			break;
		};
	}

	return 0;
}

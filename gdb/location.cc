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

int gdb_location_t::result_to_location(gdb_result_t* result, void** loc_){
	gdb_result_t* r;
	gdb_location_t* loc;


	if(*loc_ == 0)
		*loc_ = new gdb_location_t;

	loc = (gdb_location_t*)*loc_;

	list_for_each(result, r){
		switch(r->var_id){
		case IDV_LINE:
			loc->line = atoi((char*)r->value->value);
			break;

		case IDV_FILE:
			delete loc->filename;
			loc->filename = (char*)r->value->value;
			r->value->value = 0;
			break;

		case IDV_FULLNAME:
			delete loc->fullname;
			loc->fullname = (char*)r->value->value;
			r->value->value = 0;
			break;

		default:
			break;
		};
	}

	return 0;
}

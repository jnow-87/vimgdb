#include <common/list.h>
#include <gdb/breakpoint.h>
#include <stdlib.h>
#include <string.h>
#include <common/log.h>


gdb_breakpoint_t::gdb_breakpoint_t(){
	filename = 0;
	fullname = 0;
	at = 0;
	condition = 0;
	ignore_cnt = 0;
	temporary = false;
}

gdb_breakpoint_t::~gdb_breakpoint_t(){
	delete filename;
	delete fullname;
	delete at;
	delete condition;
	delete ignore_cnt;
}

int gdb_breakpoint_t::result_to_brkpt(gdb_result_t* result, void** _bkpt){
	gdb_result_t* r;
	gdb_breakpoint_t* bkpt;


	if(*_bkpt == 0)
		*_bkpt = new gdb_breakpoint_t;

	bkpt = (gdb_breakpoint_t*)*_bkpt;

	list_for_each(result, r){
		switch(r->var_id){
		case IDV_BREAKPT:
			return result_to_brkpt((gdb_result_t*)r->value->value, _bkpt);
			break;

		case IDV_NUMBER:
			bkpt->num = atoi((char*)r->value->value);
			break;

		case IDV_LINE:
			bkpt->line = atoi((char*)r->value->value);
			break;

		case IDV_FILE:
			delete bkpt->filename;
			bkpt->filename = (char*)r->value->value;
			r->value->value = 0;
			break;

		case IDV_FULLNAME:
			delete bkpt->fullname;
			bkpt->fullname = (char*)r->value->value;
			r->value->value = 0;
			break;

		case IDV_ENABLED:
			bkpt->enabled = (strcmp((const char*)r->value->value, "y") == 0) ? true : false;
			break;

		case IDV_AT:
			delete bkpt->at;
			bkpt->at = (char*)r->value->value;
			r->value->value = 0;
			break;

		case IDV_CONDITION:
			delete bkpt->condition;
			bkpt->condition = (char*)r->value->value;
			r->value->value = 0;
			break;
		
		case IDV_IGNORE:
			delete bkpt->ignore_cnt;
			bkpt->ignore_cnt = (char*)r->value->value;
			r->value->value = 0;
			break;

		case IDV_DISPOSITION:
			if(strcmp((char*)r->value->value, "del") == 0)	bkpt->temporary = true;
			else											bkpt->temporary = false;
			break;

		case IDV_TYPE:
		case IDV_ADDRESS:
		case IDV_FUNCTION:
		case IDV_ORIG_LOCATION:
		case IDV_TIMES:
		case IDV_THREAD_GROUPS:
			break;

		default:
			DEBUG("unhandled identifier %d\n", r->var_id);
			break;
		};
	}

	return 0;
}

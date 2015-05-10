#include <common/log.h>
#include <common/list.h>
#include <gdb/memory.h>
#include <stdlib.h>


gdb_memory_t::gdb_memory_t(){
	begin = 0;
	content = 0;
	length = 0;
	expanded = true;
	next = 0;
	prev = 0;
}

gdb_memory_t::~gdb_memory_t(){
	delete begin;
	delete content;
}

int gdb_memory_t::result_to_memory(gdb_result_t* result, void** _mem){
	gdb_result_t* r;
	gdb_memory_t* mem;


	if(*_mem == 0)
		*_mem = new gdb_memory_t;

	mem = (gdb_memory_t*)*_mem;

	if(result->var_id != IDV_MEMORY)
		return -1;

	list_for_each((gdb_result_t*)(((gdb_value_t*)(result->value->value))->value), r){
		switch(r->var_id){
		case IDV_BEGIN:
			delete mem->begin;
			mem->begin = (char*)r->value->value;
			r->value->value = 0;
			break;

		case IDV_END:
			mem->length = (unsigned int)(strtoll((const char*)r->value->value, 0, 16) - strtoll(mem->begin, 0, 16));
			break;

		case IDV_CONTENTS:
			delete mem->content;
			mem->content = (char*)r->value->value;
			r->value->value = 0;
			break;

		case IDV_OFFSET:
			break;

		default:
			DEBUG("unhandled identifier %d\n", r->var_id);
			break;
		};
	}

	return 0;
}

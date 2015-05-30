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
	delete [] begin;
	delete [] content;
}

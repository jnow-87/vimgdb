#include <common/list.h>
#include <gdb/breakpoint.h>
#include <stdlib.h>
#include <string.h>


gdb_breakpoint_t::gdb_breakpoint_t(){
	filename = 0;
	fullname = 0;
	at = 0;
	condition = 0;
	ignore_cnt = 0;
	temporary = false;
}

gdb_breakpoint_t::~gdb_breakpoint_t(){
	delete [] filename;
	delete [] fullname;
	delete [] at;
	delete [] condition;
	delete [] ignore_cnt;
}

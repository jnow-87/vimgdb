#include <gdb/string.h>


gdb_strlist_t::gdb_strlist_t(){
	s = 0;
	next = 0;
	prev = 0;
}

gdb_strlist_t::gdb_strlist_t(char* s){
	this->s = s;
	next = 0;
	prev = 0;
}

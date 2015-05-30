#include <common/list.h>
#include <gdb/location.h>
#include <stdlib.h>
#include <string.h>


gdb_location_t::gdb_location_t(){
	fullname = 0;
	filename = 0;
}

gdb_location_t::~gdb_location_t(){
	delete [] fullname;
	delete [] filename;
}

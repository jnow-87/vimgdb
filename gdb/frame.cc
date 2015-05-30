#include <common/list.h>
#include <gdb/frame.h>
#include <string.h>
#include <stdlib.h>


gdb_frame_t::gdb_frame_t(){
	function = 0;
	filename = 0;
	fullname = 0;
	expanded = false;
	next = 0;
	prev = 0;
}

gdb_frame_t::~gdb_frame_t(){
	list<gdb_variable_t*>::iterator it;


	delete [] function;
	delete [] filename;
	delete [] fullname;

	for(it=args.begin(); it!=args.end(); ){
		gdb_variable_t::release(*it);
		it = args.erase(it);
	}

	for(it=locals.begin(); it!=locals.end(); ){
		gdb_variable_t::release(*it);
		it = locals.erase(it);
	}
}

#include <common/list.h>
#include <common/map.h>
#include <gdb/frame.h>
#include <string.h>
#include <stdlib.h>


/* static variables */
map<string, gdb_frame_t*> gdb_frame_lst;


/* class */
gdb_frame_t::gdb_frame_t(){
	function = 0;
	filename = 0;
	fullname = 0;
	context = 0;
	expanded = false;
	next = 0;
	prev = 0;
}

gdb_frame_t::~gdb_frame_t(){
	list<gdb_variable_t*>::iterator it;


	if(context)
		MAP_ERASE(gdb_frame_lst, context);

	delete [] function;
	delete [] filename;
	delete [] fullname;
	delete [] context;

	for(it=args.begin(); it!=args.end(); ){
		if(gdb_variable_t::release(*it) != 0)
			return;

		it = args.erase(it);
	}

	for(it=locals.begin(); it!=locals.end(); ){
		if(gdb_variable_t::release(*it) != 0)
			return;

		it = locals.erase(it);
	}
}

gdb_frame_t *gdb_frame_t::acquire(){
	return new gdb_frame_t;
}

gdb_frame_t *gdb_frame_t::acquire(char *name, char *context, gdb_frame_t *src){
	gdb_frame_t *frame;
	string key;


	key = context;
	key += ":";
	key += name;

	frame = MAP_LOOKUP(gdb_frame_lst, key);

	if(frame)
		return frame;

	gdb_frame_lst[key] = src;

	src->context = new char[key.length() + 1];
	strcpy(src->context, key.c_str());

	return src;
}

void gdb_frame_t::release(gdb_frame_t *f){
	delete f;
}

#include <gdb/event.h>



#include <stdio.h>

gdb_event_t::gdb_event_t(){
	reason = 0;
	thread_id = 0;

	printf("ctor gdb_event\n");
}

gdb_event_t::~gdb_event_t(){
	delete reason;
	printf("dtor gdb_event\n");
}

gdb_event_stop_t::gdb_event_stop_t(){
	frame = 0;
	printf("ctor gdb_event_stop\n");
}

gdb_event_stop_t::~gdb_event_stop_t(){
	delete frame;
	printf("dtor gdb_event_stop\n");
}

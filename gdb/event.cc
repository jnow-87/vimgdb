#include <gdb/event.h>


gdb_event_t::gdb_event_t(){
	reason = 0;
	thread_id = 0;
}

gdb_event_t::~gdb_event_t(){
	delete [] reason;
}

gdb_event_stop_t::gdb_event_stop_t(){
	frame = 0;
}

gdb_event_stop_t::~gdb_event_stop_t(){
	delete frame;
}

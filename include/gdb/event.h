#ifndef GDB_EVENT_H
#define GDB_EVENT_H


#include <gdb/frame.h>


class gdb_event_t{
public:
	gdb_event_t();
	virtual ~gdb_event_t();

	char* reason;
	unsigned int thread_id;
};

class gdb_event_stop_t : public gdb_event_t{
public:
	gdb_event_stop_t();
	~gdb_event_stop_t();

	gdb_frame_t* frame;
};


#endif // GDB_EVENT_H

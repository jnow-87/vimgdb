#ifndef GDB_H
#define GDB_H


#include "pty.h"


/* marcos */
#define GDB_CMD		"/usr/bin/gdb"
#define GDB_ARGS	"-q"


/* class */
class gdb_if{
public:
	/* constructor/desctructor */
	gdb_if();
	~gdb_if();

	/* init gdb interface */
	int init();

	/* communication with gdb */
	int read(void* buf, unsigned int nbytes);
	int write(void* buf, unsigned int nbytes);

private:
	pty* child_term;	// PTX to gdb child
};


#endif

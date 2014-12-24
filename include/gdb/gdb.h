#ifndef GDB_H
#define GDB_H


#include <common/pty.h>


/* marcos */
#define GDB_CMD		"/usr/bin/gdb"
#define GDB_ARGS						/* specified as comma-separated list of strings */ \
					"--interpreter=mi", /* enable MI interface */ \
					"-q" 				/* disbale gdb info on start*/


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

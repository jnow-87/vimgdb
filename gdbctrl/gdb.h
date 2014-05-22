#ifndef GDB_H
#define GDB_H


#include "pty.h"


/* marcos */
#define GDB_CMD		"/usr/bin/gdb"
#define GDB_ARGS	"-q"


/* class */
class gdb_if{
public:
	gdb_if();
	~gdb_if();

	/**
	 * \brief	exec gdb and initialise its controlling terminal
	 *
	 * \return	0	on success
	 * 			-1	on error (check errno)
	 */
	int init();

private:
	pty* child_term;	// PTX to gdb child
};


#endif

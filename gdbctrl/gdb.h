#ifndef GDB_H
#define GDB_H


/* marcos */
#define GDB_CMD		"/usr/bin/gdb"
#define GDB_ARGS	"-q"


/* prototypes */
/**
 * \brief	exec gdb and initialise its controlling terminal
 *
 * \return	0	on success
 * 			-1	on error (check errno)
 */
int gdb_init();

/**
 * \brief	safely shutdown gdb session
 *
 * \param	fd	file descriptor to gdb terminal
 */
void gdb_cleanup();


#endif

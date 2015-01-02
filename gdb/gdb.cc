#include <common/log.h>
#include <common/pty.h>
#include <gdb/gdb.h>


/* class definition */
/**
 * \brief	standard constructor
 */
gdb_if::gdb_if(){
	this->child_term = 0;
}

/**
 * \brief	standard desctructor
 */
gdb_if::~gdb_if(){
	// close gdb terminal
	delete this->child_term;
	this->child_term = 0;
}

/**
 * \brief	exec gdb and initialise its controlling terminal
 *
 * \return	0	on success
 * 			-1	on error (check errno)
 */
int gdb_if::init(){
	int pid;


	// initialise pseudo terminal
	this->child_term = new pty();

	// fork child process
	pid = child_term->fork();

	if(pid == 0){
		/* child */
		log::cleanup();

		return libc::execl(GDB_CMD, GDB_CMD, GDB_ARGS, (char*)0);
	}
	else if(pid > 0){
		/* parent */
		return 0;
	}
	else{
		/* error */
		return -1;
	}
}

/**
 * \brief	read from gdb terminal
 *
 * \param	buf		target buffer
 * \param	nbytes	max bytes to read
 *
 * \return	number of read bytes on success
 * 			-1 on error
 */
int gdb_if::read(void* buf, unsigned int nbytes){
	return child_term->read(buf, nbytes);
}

/**
 * \brief	write to gdb terminal
 *
 * \param	buf		source buffer
 * \param	nbytes	number of bytes to write
 *
 * \return	number of written bytes on success
 * 			-1 on error
 */
int gdb_if::write(void* buf, unsigned int nbytes){
	return child_term->write(buf, nbytes);
}

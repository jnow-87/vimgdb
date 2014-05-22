#include "gdb.h"
#include "log.h"
#include "pty.h"


/* global functions */
gdb_if::gdb_if(){
	this->child_term = 0;
}

gdb_if::~gdb_if(){
	INFO("cleaning up gdb child\n");

	// close gdb terminal
	delete this->child_term;
	this->child_term = 0;
}

int gdb_if::init(){
	int pid;


	INFO("initialising gdb child\n");

	// initialise pseudo terminal
	this->child_term = new pty();

	// fork child process
	pid = child_term->forkpty();

	if(pid == 0){
		/* child */
		log::cleanup();

		return execl(GDB_CMD, GDB_CMD, GDB_ARGS, (char*)0);
	}
	else if(pid > 0){
		/* parent */
		return 0;
	}
	else{
		/* error */
		ERROR("forkpty failed\n");
		return -1;
	}
}

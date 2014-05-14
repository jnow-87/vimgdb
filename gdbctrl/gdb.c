#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include "gdb.h"
#include "log.h"
#include "pty.h"


/* static prototpyes */
static void sig_hdlr_chld(int signum);


/* static variables */
static int gdb_pid = -1,	// pid of gdb process
		   gdb_fd = -1;		// file descriptor to gdb terminal


/* global functions */
int gdb_init(){
	INFO("initialising gdb child\n");

	gdb_pid = forkpty(&gdb_fd, 0, 0);

	if(gdb_pid == 0){
		/* child */
		log_cleanup();

		return execl(GDB_CMD, GDB_CMD, GDB_ARGS, (char*)0);
	}
	else if(gdb_pid > 0){
		/* parent */
		// register signal handler for SIGCHLD
		if(signal(SIGCHLD, sig_hdlr_chld) == SIG_ERR){
			ERROR("register sig_hdlr_chld() failed\n");
			gdb_cleanup();

			return -1;
		}

		return 0;
	}
	else{
		/* error */
		ERROR("forkpty failed\n");
		return -1;
	}
}

void gdb_cleanup(){
	INFO("cleaning up gdb child\n");
	
	// killing gdb child process
	if(gdb_pid != -1){
		// prevent SIGCLD from being triggered
		signal(SIGCHLD, 0);

		kill(gdb_pid, SIGTERM);
		waitpid(gdb_pid, 0, 0);
		gdb_pid = -1;
	}

	// close gdb terminal
	if(gdb_fd != -1){
		close(gdb_fd);
		gdb_fd = -1;
	}
}


/* static functions */
void sig_hdlr_chld(int signum){
	INFO("caught gdb child signal %d, initialising cleanup\n", signum);
	kill(getpid(), SIGTERM);
}

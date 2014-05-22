#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "config.h"
#include "log.h"
#include "gdb.h"


/* static variables */
gdb_if* gdb;

/* static prototypes */
void cleanup(int signum);


int main(int argc, char** argv){
	/* initialise */
	// signals
	signal(SIGTERM, cleanup);
	signal(SIGINT, cleanup);

	// logging
	if(log::init(LOG_FILE, LOG_LEVEL) != 0)
		return 1;

	INFO("initialise gdbctrl\n");

	gdb = new gdb_if;

	// gdb
	if(gdb->init() != 0)
		return 2;

	/* main loop */
	// TODO
	while(1){
		sleep(1);
	}
}


/* static functions */
void cleanup(int signum){
	INFO("received signal %d\n", signum);

	delete gdb;
	log::cleanup();

	exit(1);
}

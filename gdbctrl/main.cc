#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include "config.h"
#include "log.h"
#include "gdb.h"
#include "tty.h"


/* static variables */
gdb_if* gdb;
tty std_term;


/* static prototypes */
void cleanup(int signum);
void* thread(void*);


int main(int argc, char** argv){
	pthread_t tid;


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
	INFO("initialising gdb interface\n");
	if(gdb->init() != 0)
		return 2;

	pthread_create(&tid, 0, thread, 0);

	/* main loop */
	// TODO
	char c, line[1024];
	unsigned int i = 0;

	while(1){
		std_term.read(&c, 1);

		if(c == '\n' || c == '\r'){
			line[i++] = '\n';
			line[i] = 0;
			printf("line: %s", line);
			gdb->write(line, strlen(line));

			i = 0;
		}
		else
			line[i++] = c;
	}
}


/* static functions */
void cleanup(int signum){
	INFO("received signal %d\n", signum);

	delete gdb;
	log::cleanup();

	exit(1);
}

void* thread(void* arg){
	char c, line[1024];
	unsigned int i = 0;


	while(1){
		if(gdb->read(&c, 1) == 1){
			if(c == '\n' || c == '\r'){
				line[i] = 0;
				printf("gdb_read: %s\n", line);
				i = 0;
			}
			else
				line[i++] = c;
		}
		else{
			INFO("gdb read shutdown\n");
			return 0;
		}
	}
}

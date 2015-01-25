#include <common/log.h>
#include <common/tty.h>
#include <gdb/gdb.h>
#include <gui/gui.h>
#include <gui/curses.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include "config.h"


/* static variables */
gdb_if* gdb;
tty std_term;
gui* ui;


/* static prototypes */
void cleanup(int signum);
void* thread(void*);


int main(int argc, char** argv){
	pthread_t tid;


	/* initialise */
	// signals
	signal(SIGTERM, cleanup);
	signal(SIGINT, cleanup);

	// user interface
	ui = new curses();
	ui->init();

	// logging
	if(log::init(LOG_FILE, LOG_LEVEL, ui) != 0)
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

	ui->cmd_print(CMD_PROMPT);

	while(1){
		std_term.read(&c, 1);

		if(c == '\n' || c == '\r'){
			line[i] = 0;
			gdb->exec_user_cmd(line);
			ui->cmd_print("\n" CMD_PROMPT);

			i = 0;
		}
		else if(c == 127){
			if(i <= 0)
				continue;

			line[--i] = 0;
			ui->cmd_clrline();
			ui->cmd_print(CMD_PROMPT "%s", line);
		}
		else{
			ui->cmd_print("%c", c);
			line[i++] = c;
		}
	}
}


/* static functions */
void cleanup(int signum){
	char c;


	std_term.read(&c, 1);

	delete gdb;
	log::cleanup();
	ui->destroy();
	delete (curses*)ui;

	exit(1);
}

void* thread(void* arg){
	char c, line[1024];
	unsigned int i = 0;


	while(1){
		if(gdb->read(&c, 1) == 1){
			if(c == '\n' || c == '\r'){
				if(i == 0)
					continue;

				line[i] = 0;
				ui->gdblog_print("gdb_read: %s\n", line);
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

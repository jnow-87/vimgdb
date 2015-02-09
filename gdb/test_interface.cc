#include <common/log.h>
#include <common/tty.h>
#include <gdb/gdb.h>
#include <cmd/cmd.h>
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
pthread_t tid_gdb_output,
		  tid_main;


/* static prototypes */
void cleanup(int signum);
void* thread_gdb_output(void*);


int main(int argc, char** argv){
	char c, *line;
	unsigned int i, len;


	/* initialise */
	// signals
	signal(SIGTERM, cleanup);
	signal(SIGINT, cleanup);

	// user interface
	ui = new curses();
	ui->init();

	// logging
	if(log::init(LOG_FILE, LOG_LEVEL) != 0)
		return 1;

	INFO("initialise gdbctrl\n");

	gdb = new gdb_if;

	// gdb
	INFO("initialising gdb interface\n");
	if(gdb->init() != 0)
		return 2;

	tid_main = pthread_self();
	pthread_create(&tid_gdb_output, 0, thread_gdb_output, 0);

	/* main loop */
	// TODO
	i = 0;

	len = 255;
	line = (char*)malloc(len * sizeof(char));

	if(line == 0)
		goto err;

	ui->print(WIN_CMD, CMD_PROMPT);

	while(1){
		std_term.read(&c, 1);

		if(c == '\n' || c == '\r'){
			line[i] = 0;
			cmd_exec(line, gdb);
			ui->print(WIN_CMD, "\n" CMD_PROMPT);

			i = 0;
		}
		else if(c == 127){
			if(i <= 0)
				continue;

			line[--i] = 0;
			ui->clearline(WIN_CMD);
			ui->print(WIN_CMD, CMD_PROMPT "%s", line);
		}
		else{
			ui->print(WIN_CMD, "%c", c);
			line[i++] = c;

			if(i >= len){
				len *= 2;
				line = (char*)realloc(line, len);

				if(line == 0)
					goto err;
			}
		}
	}

err:
	pthread_cancel(tid_gdb_output);
	cleanup(SIGTERM);
}


/* static functions */
void cleanup(int signum){
	char c;


	pthread_join(tid_gdb_output, 0);

	std_term.read(&c, 1);

	delete gdb;

	log::cleanup();
	ui->destroy();
	delete (curses*)ui;

	exit(1);
}

void* thread_gdb_output(void* arg){
	char c, *line;
	unsigned int i, len;
	sigval v;


	i = 0;

	len = 255;
	line = (char*)malloc(len * sizeof(char));

	if(line == 0)
		goto err;

	while(1){
		if(gdb->read(&c, 1) == 1){
			// ignore CR to avoid issues when printing the string
			if(c == '\r')
				continue;

			line[i++] = c;

			if(i >= len){
				len *= 2;
				line = (char*)realloc(line, len);

				if(line == 0)
					goto err;
			}

			// check for end of gdb line, a simple newline as separator
			// doesn't work, since the parse would try to parse the line,
			// detecting a syntax error
			if(strncmp(line + i - 6, "(gdb)\n", 6) == 0 ||
			   strncmp(line + i - 7, "(gdb) \n", 7) == 0
			  ){
				line[i] = 0;
				ui->print(WIN_GDBLOG, "gdb_read: %s", line);

				TEST("parse gdb string \"%.10s...\"\n", line);
				TEST("parser return value: %d\n", gdb->mi_parse(line));

				i = 0;
			}
		}
		else{
			INFO("gdb read shutdown\n");
			pthread_exit(0);
		}
	}

err:
	pthread_sigqueue(tid_main, SIGTERM, v);
	pthread_exit(0);
}

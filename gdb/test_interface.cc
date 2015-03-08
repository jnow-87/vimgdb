#include <common/log.h>
#include <common/tty.h>
#include <common/opt.h>
#include <gdb/gdb.h>
#include <gdb/parser.tab.h>
#include <user_cmd/cmd.h>
#include <gui/gui.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>

#ifdef GUI_CURSES
	#include <gui/curses/cursesui.h>
#elif GUI_VIM
	#include <gui/vim/vimui.h>
#endif


/* static variables */
gdbif* gdb;
pthread_t tid_gdb_output,
		  tid_main;


/* static prototypes */
void cleanup(int signum);
void* thread_gdb_output(void*);


int main(int argc, char** argv){
	char* line;


	/* initialise */
	if(opt_parse(argc, argv) != 0)
		return 1;

	// signals
	signal(SIGTERM, cleanup);
	signal(SIGINT, cleanup);

	// user interface
#ifdef GUI_CURSES
	ui = new cursesui();
#elif GUI_VIM
	ui = new vimui();
#else
	#error "invalid gui defined"
#endif

	if(ui->init() != 0)
		return 1;

	// logging
	if(log::init(LOG_FILE, LOG_LEVEL) != 0)
		return 1;

	// gdb
	INFO("initialise gdbctrl\n");
	gdb = new gdbif;

	INFO("initialising gdb interface\n");
	if(gdb->init() != 0)
		return 2;

	tid_main = pthread_self();
	pthread_create(&tid_gdb_output, 0, thread_gdb_output, 0);

	/* main loop */
	while(1){
		line = ui->readline();

		if(line == 0){
			ERROR("error reading user input\n");
			goto err;
		}

		cmd_exec(line, gdb);
	}

err:
	pthread_cancel(tid_gdb_output);
	cleanup(SIGTERM);
}


/* static functions */
void cleanup(int signum){
	char c;


	pthread_join(tid_gdb_output, 0);

	delete gdb;

	log::cleanup();
	ui->destroy();

#ifdef GUI_CURSES
	delete (cursesui*)ui;
#elif GUI_VIM
	delete (vimui*)ui;
#else
	#error "invalid gui defined"
#endif

	exit(1);
}

void* thread_gdb_output(void* arg){
	char c, *line;
	int win_id_gdb;
	unsigned int i, len;
	sigval v;


	i = 0;

	len = 255;
	line = (char*)malloc(len * sizeof(char));

	if(line == 0)
		goto err_0;

	win_id_gdb = ui->win_create("gdb-log", true, 0);

	if(win_id_gdb < 0)
		goto err_1;

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
					goto err_0;
			}

			// check for end of gdb line, a simple newline as separator
			// doesn't work, since the parse would try to parse the line,
			// detecting a syntax error
			if(strncmp(line + i - 6, "(gdb)\n", 6) == 0 ||
			   strncmp(line + i - 7, "(gdb) \n", 7) == 0
			  ){
				line[i] = 0;
				ui->win_print(win_id_gdb, "gdb_read: %s", line);

				TEST("parse gdb string \"%.10s...\"\n", line);
				TEST("parser return value: %d\n", gdbparse(line, gdb));

				i = 0;
			}
		}
		else{
			INFO("gdb read shutdown\n");
			goto err_2;
		}
	}

err_2:
	ui->win_destroy(win_id_gdb);

err_1:
	free(line);

err_0:
	pthread_sigqueue(tid_main, SIGTERM, v);
	pthread_exit(0);
}

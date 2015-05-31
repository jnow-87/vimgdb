#include <common/log.h>
#include <common/opt.h>
#include <gdb/gdb.h>
#include <gui/gui.h>
#include <user_cmd/cmd.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

#ifdef GUI_CURSES
	#include <gui/curses/cursesui.h>
#elif GUI_VIM
	#include <gui/vim/vimui.h>
#endif


/* static prototypes */
void cleanup(int signum);
int cleanup();


int main(int argc, char** argv){
	char* line;


	/* initialise */
	thread_name[pthread_self()] = "main";

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
	DEBUG("initialise gdbctrl\n");
	gdb = new gdbif;

	DEBUG("initialising gdb interface\n");
	if(gdb->init() != 0)
		return 2;

	gdb->on_stop(cmd_var_print);
	gdb->on_stop(cmd_callstack_update);
	gdb->on_stop(cmd_register_print);
	gdb->on_stop(cmd_memory_update);
	gdb->on_exit(cleanup);

	/* main loop */
	while(1){
		line = ui->readline();

		if(line == 0){
			DEBUG("detect ui shutdown\n");
			goto end;
		}

		cmd_exec(line);
	}

end:
	// call cleanup() through signal to prevent nested signals
	// from other threads like closing ui and gdb
	cleanup();
}


/* static functions */
void cleanup(int signum){
	cleanup();
}

int cleanup(){
	static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;


	DEBUG("recv cleanup request\n");

	/* ensure cleanup is only executed once */
	if(pthread_mutex_trylock(&m) != 0)
		return 0;

	/* destroy gdb */
	DEBUG("close gdb interface\n");
	delete gdb;
	gdb = 0;

	/* close log */
	DEBUG("close log\n");
	log::cleanup();

	/* close gui */
	if(ui)
		ui->destroy();

	delete ui;
	ui = 0;
	
	pthread_mutex_unlock(&m);

	exit(0);
}

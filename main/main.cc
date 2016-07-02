#include <config/config.h>
#include <common/log.h>
#include <common/opt.h>
#include <gdb/gdb.h>
#include <gui/gui.h>
#include <user_cmd/cmd.h>
#include <user_cmd/cmd.hash.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>

#ifdef CONFIG_GUI_CURSES
	#include <gui/curses/cursesui.h>
#elif CONFIG_GUI_VIM
	#include <gui/vim/vimui.h>
#endif


/* static prototypes */
int cleanup();
void cleanup(int signum);


int main(int argc, char **argv){
	char *line;
	char log_name[strlen(CONFIG_LOG_FILE) + 10];
	int n;


	/* initialise */
	thread_name[pthread_self()] = "main";

	if(opt_parse(argc, argv) != 0)
		return 1;

	// signals
	signal(SIGTERM, cleanup);
	signal(SIGINT, cleanup);

	// user interface
#ifdef CONFIG_GUI_CURSES
	ui = new cursesui();
#elif CONFIG_GUI_VIM
	ui = new vimui();
#else
	#error "invalid gui defined"
#endif

	for(n=0; n<10; n++){
		if(ui->init() == 0)
			break;

		printf("unable to initialise user interface, trying again\n");
		sleep(1);
	}

	if(n == 10){
		printf("attempt to initialise user interface failed %d times, giving up\n", n);
		cleanup(1);
	}

	// logging
	if(opt.log_file)
		strcpy(log_name, opt.log_file);
	else
		sprintf(log_name, CONFIG_LOG_FILE ".%d", getpid());

	if(log::init(log_name, LOG_LEVEL) != 0)
		cleanup(1);

	// gdb
	DEBUG("initialise gdbctrl\n");

	gdb = new gdbif;

	DEBUG("initialising gdb interface\n");

	if(gdb->init() != 0){
		ERROR("initialising gdb interface\n");
		cleanup(2);
	}

	gdb->on_stop(cmd_var_print);
	gdb->on_stop(cmd_callstack_update);
	gdb->on_stop(cmd_register_print);
	gdb->on_stop(cmd_memory_update);
	gdb->on_stop(cmd_per_update);
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
	cleanup(0);
}


/* static functions */
int cleanup(){
	cleanup(0);
	return 0;
}

void cleanup(int signum){
	static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
	unsigned int i;


	DEBUG("recv cleanup request\n");

	/* ensure cleanup is only executed once */
	if(pthread_mutex_trylock(&m) != 0){
		DEBUG("cleanup already ongoing, nothing to be done\n");
		return;
	}

	DEBUG("processing cleanup request\n");

	/* call user-command cleanup functions */
	for(i=user_cmd::MIN_HASH_VALUE; i<=user_cmd::MAX_HASH_VALUE; i++){
		if(user_cmd::wordlist[i].name[0] != 0 && user_cmd::wordlist[i].cleanup != 0){
			DEBUG("calling user-command cleanup for %s\n", user_cmd::wordlist[i].name);
			user_cmd::wordlist[i].cleanup();
		}
	}

	/* destroy gdb */
	DEBUG("destroying gdb interface\n");

	delete gdb;
	gdb = 0;

	/* close gui */
	DEBUG("destroying gui\n");

	if(ui)
		ui->destroy();

	delete ui;
	ui = 0;

	/* close log */
	DEBUG("closing log and exit normally\n");
	log::cleanup();

	pthread_mutex_unlock(&m);

	exit(signum);
}

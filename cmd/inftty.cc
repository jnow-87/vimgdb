#include <common/log.h>
#include <common/pty.h>
#include <gdb/gdb.h>
#include <gdb/result.h>
#include <gui/gui.h>
#include <cmd/cmd.h>
#include <cmd/subcmd.hash.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>


/* static variables */
static pty* inferior_term = 0;
static pthread_t tid;


/* static prototypes */
static void* thread_inferior_output(void* arg);


int cmd_inftty_exec(gdb_if* gdb, int argc, char** argv){
	char* pty_name;
	int fd;


	/* check arguments */
	if(argc != 2){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_inftty_help(argv[0]);
		return -1;
	}

	/* setup pty */
	if(strcmp(argv[1], "internal") == 0){
		/* setup pty for output redirection */
		// return if inferior tty is already set to internal
		if(inferior_term != 0)
			return 0;
		
		// initialise pty
		inferior_term = new pty;
		if(inferior_term == 0){
			WARN("unable to allocate new pty\n");
			return -1;
		}

		pty_name = inferior_term->get_name();

		// start thread to read from the pty
		if(pthread_create(&tid, 0, thread_inferior_output, 0) != 0){
			WARN("unable to create thread\n");

			delete inferior_term;
			inferior_term = 0;

			return -1;
		}
	}
	else{
		/* use specified pty for output */
		fd = open(argv[1], O_RDONLY);
		if(fd == -1){
			USER("unable to open pts \"%s\"\n", argv[1]);
			return -1;
		}

		close(fd);

		pty_name = argv[1];
	}

	/* issue MI command */
	if(gdb->mi_issue_cmd((char*)"inferior-tty-set", 0, 0, &pty_name, 1, cmd_inftty_resp) < 0){
		WARN("error sending mi command\n");
		return -1;
	}

	return 0;
}

int cmd_inftty_resp(result_class_t rclass, result_t* result, char* cmdline, void* data){
	switch(rclass){
	case RC_DONE:
		USER("done: exec \"%s\"\n", cmdline);
		break;

	default:
		WARN("unhandled result class %d\n", rclass);
		break;
	};

	return 0;
}

void cmd_inftty_help(char* cmd){
	USER("usage %s <terminal>\n", cmd);
	USER("   <terminal> is either\n");
	USER("      a pseudo terminal in the form of \"/dev/pts/1\", or\n");
	USER("      'internal' which redirects the output to one of the gui windows\n");
}


/* local functions */
void* thread_inferior_output(void* arg){
	char c, line[1024];
	unsigned int i;


	i = 0;
	while(1){
		if(inferior_term->read(&c, 1) == 1){
			// ignore CR to avoid issues when printing the string
			if(c == '\r')
				continue;

			line[i++] = c;

			if(c == '\n'){
				line[i] = 0;
				ui->print(WIN_INFERIOR, line);
				i = 0;
			}

		}
		else{
			INFO("inferior read shutdown\n");
			return 0;
		}
	}

}

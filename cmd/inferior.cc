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


/* global functions */
int cmd_inferior_exec(gdb_if* gdb, int argc, char** argv){
	const char* cmd_str;
	const struct subcmd_t* scmd;
	char** argp;
	char* pty_name;
	int fd;


	if(argc < 2){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_inferior_help(argv[0]);
		return -1;
	}

	scmd = subcmd::lookup(argv[1], strlen(argv[1]));
	if(scmd != 0){
		switch(scmd->id){
		case BIN:
			cmd_str = "file-exec-file";
			argp = argv + 2;
			argc -= 2;
			break;

		case SYM:
			cmd_str = "file-symbol-file";
			argp = argv + 2;
			argc -= 2;
			break;

		case ARGS:
			cmd_str = "exec-arguments";
			argp = argv + 2;
			argc -= 2;
			break;

		case TTY:
			/* setup pty */
			if(strcmp(argv[2], "internal") == 0){
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
				fd = open(argv[2], O_RDONLY);
				if(fd == -1){
					USER("unable to open pts \"%s\"\n", argv[2]);
					return -1;
				}

				close(fd);

				pty_name = argv[2];
			}

			cmd_str = "inferior-tty-set";
			argp = &pty_name;
			argc = 1;

			break;

		default:
			USER("invalid sub command \"%s\" to \"%s\"\n", argv[1], argv[0]);
			return -1;
		};
	}
	else{
		cmd_str = "file-exec-and-symbols";
		argp = argv + 1;
		argc = 1;
	}

	if(gdb->mi_issue_cmd((char*)cmd_str, 0, 0, argp, argc, cmd_inferior_resp) < 0){
		WARN("error sending mi command\n");
		return -1;
	}

	return 0;
}

int cmd_inferior_resp(result_class_t rclass, result_t* result, char* cmdline, void* data){
	switch(rclass){
	case RC_DONE:
		USER("done: exec \"%s\"\n", cmdline);
		break;

	case RC_ERROR:
		USER("gdb reported error for command \"%s\"\n\t%s\n", cmdline, result->value->value);
		break;

	default:
		WARN("unhandled result class %d\n", rclass);
		break;
	};

	return 0;
}

void cmd_inferior_help(char* cmd){
	USER("usage: %s [sub-command] <args>...\n", cmd);
	USER("   sub-commands:\n");
	USER("      <empty>   treat <args> as input binary and symbol file\n");
	USER("      bin       load only code from <args>\n");
	USER("      sym       load debug symbols from <args>\n");
	USER("      args      set inferior parameter for next run to <args>\n");
	USER("      tty       set inferior output terminal, <args> is either\n");
	USER("                   a pseudo terminal in the form of \"/dev/pts/1\", or\n");
	USER("                   'internal' which redirects the output to one of the gui windows\n");
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

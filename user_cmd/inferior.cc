#include <common/log.h>
#include <common/pty.h>
#include <gdb/gdb.h>
#include <gdb/result.h>
#include <gui/gui.h>
#include <user_cmd/cmd.h>
#include <user_cmd/subcmd.hash.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>


/* static variables */
static pty* inferior_term = 0;
static pthread_t tid;


/* static prototypes */
static void* thread_inferior_output(void* arg);


/* global functions */
int cmd_inferior_exec(gdbif* gdb, int argc, char** argv){
	int fd;
	const struct user_subcmd_t* scmd;
	gdb_response_t* resp;


	if(argc < 2){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_inferior_help(1, argv);
		return 0;
	}

	scmd = user_subcmd::lookup(argv[1], strlen(argv[1]));

	if(scmd != 0){
		if(scmd->id == BIN)			resp = gdb->mi_issue_cmd((char*)"file-exec-file", "%ss %d", argv + 2, argc - 2);
		else if(scmd->id == SYM)	resp = gdb->mi_issue_cmd((char*)"file-symbol-file", "%ss %d", argv + 2, argc - 2);
		else if(scmd->id ==  ARGS){
			TODO("implement arguments with spaces\n");
			resp = gdb->mi_issue_cmd((char*)"exec-arguments", "%ss %d", argv + 2, argc - 2);
		}
		else if(scmd->id == TTY){
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

				// start thread to read from the pty
				if(pthread_create(&tid, 0, thread_inferior_output, 0) != 0){
					WARN("unable to create thread\n");

					delete inferior_term;
					inferior_term = 0;

					return -1;
				}

				resp = gdb->mi_issue_cmd((char*)"inferior-tty-set", "%s", inferior_term->get_name());
			}
			else{
				/* use specified pty for output */
				fd = open(argv[2], O_RDONLY);
				if(fd == -1){
					USER("unable to open pts \"%s\"\n", argv[2]);
					return 0;
				}

				close(fd);

				resp = gdb->mi_issue_cmd((char*)"inferior-tty-set", "%ss %d", argv + 2, argc - 2);
			}
		}
		else{
			USER("invalid sub-command \"%s\" to command \"%s\"\n", argv[1], argv[0]);
			return 0;
		}
	}
	else{
		resp = gdb->mi_issue_cmd((char*)"file-exec-and-symbols", "%ss %d", argv + 1, argc - 1);
	}

	if(resp == 0){
		WARN("error issuing mi command\n");
		return -1;
	}

	switch(resp->rclass){
	case RC_DONE:
		USER("done: exec \"%s %s\"\n", argv[0], argv[1]);
		break;

	case RC_ERROR:
		USER("gdb reported error for command \"%s %s\"\n\t%s\n", argv[0], argv[1], resp->result->value->value);
		break;

	default:
		WARN("unhandled result class %d\n", resp->rclass);
		break;
	};

	gdb_result_free(resp->result);
	return 0;
}

void cmd_inferior_help(int argc, char** argv){
	unsigned int i;
	const struct user_subcmd_t* scmd;


	if(argc == 1){
		USER("usage: %s [sub-command] <args>...\n", argv[0]);
		USER("   sub-commands:\n");
		USER("      <file>           load code and debug symbols\n");
		USER("      bin <file>       load code\n");
		USER("      sym <file>       load debug symbols\n");
		USER("      args <args>...   set inferior parameters\n");
		USER("      tty <terminal>   set inferior output terminal\n");
		USER("\n");
	}
	else{
		for(i=1; i<argc; i++){
			scmd = user_subcmd::lookup(argv[i], strlen(argv[i]));

			if(scmd == 0){
				USER("invalid sub-command \"%s\" to command \"%s\"\n", argv[i], argv[0]);
				continue;
			}

			switch(scmd->id){
			case BIN:
				USER("usage %s %s <file>\n", argv[0], argv[i]);
				USER("   load code from <file>\n");
				USER("\n");
				break;

			case SYM:
				USER("usage %s %s <file>\n", argv[0], argv[i]);
				USER("   load debug symbols from <file>\n");
				USER("\n");
				break;

			case TTY:
				USER("usage %s %s <terminal>\n", argv[0], argv[i]);
				USER("   set inferior output terminal, <terminal> is either\n");
				USER("      a pseudo terminal in the form of \"/dev/pts/1\", or\n");
				USER("      'internal' which redirects the output to one of the gui windows\n");
				USER("\n");
				break;

			case ARGS:
				USER("usage %s %s <args>...\n", argv[0], argv[i]);
				USER("   set inferior parameters for next run to <args>\n");
				USER("\n");
				break;

			default:
				USER("invalid sub-command \"%s\" to command \"%s\"\n", argv[i], argv[0]);
			};
		}
	}
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

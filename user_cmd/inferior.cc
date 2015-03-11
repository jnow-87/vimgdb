#include <common/log.h>
#include <common/pty.h>
#include <common/list.h>
#include <gdb/gdb.h>
#include <gdb/location.h>
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
	int fd, r;
	const struct user_subcmd_t* scmd;
	gdb_location_t* loc;


	if(argc < 2){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_inferior_help(1, argv);
		return 0;
	}

	loc = 0;
	scmd = user_subcmd::lookup(argv[1], strlen(argv[1]));

	if(scmd == 0 || scmd->id == SYM){
		if(scmd == 0)	r = gdb->mi_issue_cmd((char*)"file-exec-and-symbols", RC_DONE, 0, "%ss %d", argv + 1, argc - 1);
		else			r = gdb->mi_issue_cmd((char*)"file-symbol-file", RC_DONE, 0, "%ss %d", argv + 2, argc - 2);

		if(r != 0){
			USER("error loading file\n");
			goto end;
		}

		if(gdb->mi_issue_cmd((char*)"file-list-exec-source-file", RC_DONE, (void**)&loc, "") != 0){
			USER("error getting current source file\n");
			goto end;
		}

		ui->win_create(loc->fullname);
		if(scmd == 0)	USER("loaded input file \"%s\"\n", argv[1]);
		else			USER("loaded input file \"%s\"\n", argv[2]);
	}
	else{
		if(scmd->id == BIN){
			if(gdb->mi_issue_cmd((char*)"file-exec-file", RC_DONE, 0, "%ss %d", argv + 2, argc - 2) != 0)
				USER("error loading binary file \"%s\"\n", argv[2]);
			else
				USER("loaded binary file \"%s\"\n", argv[2]);
		}
		else if(scmd->id ==  ARGS){
			TODO("implement arguments with spaces\n");
			if(gdb->mi_issue_cmd((char*)"exec-arguments", RC_DONE, 0, "%ss %d", argv + 2, argc - 2) != 0)
				USER("error setting program arguments\n");
			else
				USER("set program arguments\n");
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
					USER("error allocating pseudo terminal for debugee\n");
					return 0;
				}

				// start thread to read from the pty
				if(pthread_create(&tid, 0, thread_inferior_output, 0) != 0){
					USER("error creating thread to read inferior output\n");

					delete inferior_term;
					inferior_term = 0;

					return 0;
				}

				if(gdb->mi_issue_cmd((char*)"inferior-tty-set", RC_DONE, 0, "%s", inferior_term->get_name()) != 0)
					USER("error setting inferior tty\n");
				else
					USER("set inferior tty to internal\n");
			}
			else{
				/* use specified pty for output */
				fd = open(argv[2], O_RDONLY);
				if(fd == -1){
					USER("error opening pts \"%s\"\n", argv[2]);
					return 0;
				}

				close(fd);

				if(gdb->mi_issue_cmd((char*)"inferior-tty-set", RC_DONE, 0, "%ss %d", argv + 2, argc - 2) != 0)
					USER("error setting inferior tty to \"%s\"\n", argv[2]);
				else
					USER("set inferior tty to \"%s\"\n", argv[2]);
			}
		}
		else{
			USER("invalid sub-command \"%s\" to command \"%s\"\n", argv[1], argv[0]);
			return 0;
		}
	}

end:
	delete loc;
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
	int win_id_inf;
	unsigned int i;


	win_id_inf = ui->win_getid("inferior");

	if(win_id_inf < 0)
		return 0;

	i = 0;
	while(1){
		if(inferior_term->read(&c, 1) == 1){
			// ignore CR to avoid issues when printing the string
			if(c == '\r')
				continue;

			line[i++] = c;

			if(c == '\n'){
				line[i] = 0;
				ui->win_print(win_id_inf, line);
				i = 0;
			}

		}
		else{
			INFO("inferior read shutdown\n");

			ui->win_destroy(win_id_inf);
			return 0;
		}
	}
}

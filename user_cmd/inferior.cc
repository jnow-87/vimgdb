#include <common/defaults.h>
#include <common/log.h>
#include <common/pty.h>
#include <common/list.h>
#include <common/string.h>
#include <gdb/gdb.h>
#include <gdb/location.h>
#include <gui/gui.h>
#include <user_cmd/cmd.h>
#include <user_cmd/subcmd.hash.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>


/* macros */
#define TTY_EXT_FILE	"/tmp/vimgdb_pts"
#define TTY_EXT_CMD		"xterm -T 'vimgdb-inferior' -e 'ls  /proc/self/fd/0 -l | grep -o -e \"/dev/pts.*\" > " TTY_EXT_FILE "; while [ 0 == 0 ];do sleep 10; done' &"


/* static variables */
static char *inf_file_bin = 0,
			*inf_file_sym = 0;

static char** inf_argv = 0;
static int inf_argc = 0;

static pty* inf_term = 0;
static pthread_t tid = 0;


/* static prototypes */
static void* thread_inferior_output(void* arg);


/* global functions */
int cmd_inferior_exec(int argc, char** argv){
	int fd, r;
	char pts[128];
	const struct user_subcmd_t* scmd;
	FILE* fp;
	gdb_location_t* loc;


	if(argc < 2){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_inferior_help(1, argv);
		return 0;
	}

	scmd = user_subcmd::lookup(argv[1], strlen(argv[1]));

	if((scmd == 0 && argc < 2) || (scmd && (((scmd->id == SYM || scmd->id == BIN || scmd->id == TTY || scmd->id == ARGS) && argc < 3) || ((scmd->id == EXPORT) && argc < 4)))){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_inferior_help(2, argv);
		return 0;
	}

	if(scmd == 0 || scmd->id == SYM){
		if(scmd == 0){
			if(gdb->mi_issue_cmd("file-exec-and-symbols", 0, "%ss %d", argv + 1, argc - 1) != 0)
				return -1;

			USER("load file \"%s\"\n", argv[1]);

			delete [] inf_file_bin;
			delete [] inf_file_sym;

			inf_file_bin = stralloc(argv[1], strlen(argv[1]));
			inf_file_sym = stralloc(argv[1], strlen(argv[1]));
		}
		else{
			if(gdb->mi_issue_cmd("file-symbol-file", 0, "%ss %d", argv + 2, argc - 2) != 0)
				return -1;

			USER("load file \"%s\"\n", argv[2]);

			delete [] inf_file_sym;

			inf_file_sym = stralloc(argv[2], strlen(argv[2]));
		}

		if(gdb->mi_issue_cmd("file-list-exec-source-file", (gdb_result_t**)&loc, "") != 0)
			return -1;

		ui->win_create(loc->fullname);
		delete loc;
	}
	else{
		switch(scmd->id){
		case BIN:
			if(gdb->mi_issue_cmd("file-exec-file", 0, "%ss %d", argv + 2, argc - 2) != 0)
				return -1;

			USER("load binary file \"%s\"\n", argv[2]);

			delete [] inf_file_bin;

			inf_file_bin = stralloc(argv[2], strlen(argv[2]));
			break;

		case ARGS:
			if(gdb->mi_issue_cmd("exec-arguments", 0, "%ssq %d", argv + 2, argc - 2) != 0)
				return -1;

			USER("set program arguments\n");

			for(r=0; r<inf_argc; r++)
				delete [] inf_argv[r];
			delete [] inf_argv;

			inf_argc = argc - 2;
			inf_argv = new char*[inf_argc];

			for(r=2; r<argc; r++)
				inf_argv[r - 2] = stralloc(argv[r], strlen(argv[r]));
			break;

		case TTY:
			/* setup pty */
			if(strcmp(argv[2], "internal") == 0){
				/* setup pty for output redirection */
				// return if inferior tty is already set to internal
				if(inf_term != 0)
					return 0;
				
				// initialise pty
				inf_term = new pty;
				if(inf_term == 0){
					USER("error allocating pseudo terminal for debugee\n");
					return -1;
				}

				// start thread to read from the pty
				if(pthread_create(&tid, 0, thread_inferior_output, 0) != 0){
					USER("error creating thread to read inferior output\n");

					delete inf_term;
					inf_term = 0;

					return -1;
				}

				thread_name[tid] = "inferior";

				if(gdb->mi_issue_cmd("inferior-tty-set", 0, "%s", inf_term->get_name()) != 0)
					return -1;

				USER("set inferior tty to internal\n");
			}
			else{
				/* close internal terminal if exists */
				if(inf_term != 0){
					pthread_cancel(tid);
					pthread_join(tid, 0);

					delete inf_term;
					inf_term = 0;
				}

				/* get pseudo terminal */
				if(strcmp(argv[2], "external") == 0){
					unlink(TTY_EXT_FILE);
					system(TTY_EXT_CMD);

					// wait for xterm to echo its pts
					r = 0;
					while((fp = fopen(TTY_EXT_FILE, "r")) == 0){
						if(++r > 50){
							USER("error, waiting for pts of external tty\n");
							return -1;
						}

						usleep(100000);
					}

					// read pts
					fscanf(fp, "%128s", pts);
					fclose(fp);
				}
				else
					strncpy(pts, argv[2], 128);

				/* use specified pty for output */
				fd = open(pts, O_RDONLY);
				if(fd == -1){
					USER("error opening pts \"%s\"\n", pts);
					return -1;
				}

				close(fd);

				if(gdb->mi_issue_cmd("inferior-tty-set", 0, "%s", pts) != 0)
					return -1;

				USER("set inferior tty to \"%s\"\n", pts);
			}

			break;

		case EXPORT:
			fp = fopen(argv[2], "w");

			if(fp == 0)
				return 0;

			if(inf_file_bin)	fprintf(fp, "Inferior bin %s\n", inf_file_bin);
			if(inf_file_sym)	fprintf(fp, "Inferior sym %s\n", inf_file_sym);

			if(inf_term)		fprintf(fp, "Inferior tty internal\n");
			else				fprintf(fp, "echoerr \"inferior tty not set to internal, please adjust\"\n");

			if(inf_argc > 0)
				fprintf(fp, "Inferior args");

			for(r=0; r<inf_argc; r++)
				fprintf(fp, " \"%s\"", inf_argv[r]);

			fprintf(fp, "\n\n");
			fclose(fp);

			USER("export inferior data to \"%s\"\n", argv[2]);

			/* signal data availability */
			fp = fopen(argv[3], "w");

			if(fp == 0)
				return -1;

			fprintf(fp, "1\n");
			fclose(fp);
			break;

		default:
			USER("invalid sub-command \"%s\" to command \"%s\"\n", argv[1], argv[0]);
			return 0;
		}
	}

	if(scmd == 0 || scmd->id == BIN){
		if(cmd_register_init() == 0)	USER("initialised registers\n");
		else							USER("error initialising registers\n");
	}

	return 0;
}

void cmd_inferior_help(int argc, char** argv){
	int i;
	const struct user_subcmd_t* scmd;


	ui->atomic(true);

	if(argc == 1){
		USER("usage: %s [sub-command] <args>...\n", argv[0]);
		USER("   sub-commands:\n");
		USER("      <file>                 load code and debug symbols\n");
		USER("      bin <file>             load code\n");
		USER("      sym <file>             load debug symbols\n");
		USER("      args <args>...         set inferior parameters\n");
		USER("      tty <terminal>         set inferior output terminal\n");
		USER("      export <file> <sync>   export inferior data to vim script\n");
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
				USER("      a pseudo terminal in the form of \"/dev/pts/1\",\n");
				USER("      'internal' which redirects the output to one of the gui windows or\n");
				USER("      'external' which creates an xterm and redirects the inferior terminal to it\n");
				USER("\n");
				break;

			case ARGS:
				USER("usage %s %s <args>...\n", argv[0], argv[i]);
				USER("   set inferior parameters for next run to <args>\n");
				USER("\n");
				break;

			case EXPORT:
				USER("usage %s %s <file> <sync>\n", argv[0], argv[1]);
				USER("   export inferior data to vim script <filename>, using file <sync> to sync with vim\n");
				USER("\n");
				break;

			default:
				USER("invalid sub-command \"%s\" to command \"%s\"\n", argv[i], argv[0]);
			};
		}
	}

	ui->win_cursor_set(ui->win_getid(USERLOG_NAME), -1);
	ui->atomic(false);
}

/* local functions */
void* thread_inferior_output(void* arg){
	char c;
	char* line;
	unsigned int i, len = 256;


	line = (char*)malloc(len);

	if(line == 0)
		return 0;

	i = 0;
	while(1){
		if(inf_term->read(&c, 1) == 1){
			// ignore CR to avoid issues when printing the string
			if(c == '\r')
				continue;

			line[i++] = c;

			if(i >= len){
				len += 256;
				line = (char*)realloc(line, len * sizeof(char));

				if(line == 0)
					pthread_exit(0);
			}

			if(c == '\n'){
				line[i] = 0;

				while(ui->win_getid(INFERIOR_NAME) == -1)
					usleep(100000);

				ui->win_print(ui->win_getid(INFERIOR_NAME), line);
				i = 0;
			}

		}
		else{
			DEBUG("inferior read shutdown\n");

			ui->win_destroy(ui->win_getid(INFERIOR_NAME));
			free(line);
			pthread_exit(0);
		}
	}
}

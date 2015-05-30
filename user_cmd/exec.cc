#include <common/defaults.h>
#include <common/log.h>
#include <common/file.h>
#include <gdb/gdb.h>
#include <gdb/location.h>
#include <gui/gui.h>
#include <user_cmd/cmd.h>
#include <user_cmd/subcmd.hash.h>


/* global functions */
int cmd_exec_exec(int argc, char** argv){
	int r;
	const struct user_subcmd_t* scmd;
	gdb_location_t* loc;


	if(argc < 2){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_exec_help(1, argv);
		return 0;
	}

	loc = 0;
	scmd = user_subcmd::lookup(argv[1], strlen(argv[1]));

	if(scmd == 0){
		USER("invalid sub-command \"%s\" to command \"%s\"\n", argv[1], argv[0]);
		return 0;
	}

	if(gdb->running()){
		if(scmd->id == BREAK){
			gdb->sigsend(SIGINT);
		}
		else{
			USER("error executing \"%s\" - inferior is running\n", argv[1]);
			return 0;
		}
	}
	else{
		if(gdb->mi_issue_cmd("file-list-exec-source-file", (gdb_result_t**)&loc, "") != 0)
			return -1;

		if(FILE_EXISTS(loc->fullname))	ui->win_anno_delete(ui->win_create(loc->fullname), loc->line, "ip");
		else							USER("file \"%s\" does not exist\n", loc->fullname);

		delete loc;

		switch(scmd->id){
		case RUN:
			r = gdb->mi_issue_cmd("exec-run", 0, "");
			break;

		case NEXT:
			r = gdb->mi_issue_cmd("exec-next", 0, "");
			break;

		case STEP:
			r = gdb->mi_issue_cmd("exec-step", 0, "");
			break;

		case RETURN:
			r = gdb->mi_issue_cmd("exec-finish", 0, "--thread %u --frame 0", gdb->threadid());
			break;

		case GOTO:
			if(gdb->mi_issue_cmd("break-insert", 0, "-t %ss %d", argv + 2, argc - 2) != 0)
				return -1;
				
			r = gdb->mi_issue_cmd("exec-continue", 0, "");
			break;

		case SETPC:
			if(gdb->mi_issue_cmd("break-insert", 0, "-t %ss %d", argv + 2, argc - 2) != 0)
				return -1;
				
			r = gdb->mi_issue_cmd("exec-jump", 0, "%ss %d", argv + 2, argc - 2);
			break;

		case CONTINUE:
			r = gdb->mi_issue_cmd("exec-continue", 0, "");

			if(r != 0){
				USER("error executing \"%s\", trying to start inferior first\n", argv[1]);
				r = gdb->mi_issue_cmd("exec-run", 0, "");
			}

			break;

		case BREAK:
			USER("error executing \"%s\" - inferior is not running\n", argv[1]);
			return 0;

		default:
			r = 0;
			USER("unhandled sub command \"%s\" to \"%s\"\n", argv[1], argv[0]);
			break;
		};

		if(r != 0)
			return -1;
	}

	USER("%s\n", argv[1]);

	return 0;
}

void cmd_exec_help(int argc, char** argv){
	int i;
	const struct user_subcmd_t* scmd;


	ui->atomic(true);

	if(argc == 1){
		USER("usage: %s [sub-command] <args>...\n", argv[0]);
		USER("   sub-commands:\n");
		USER("      run               (re)start inferior execution\n");
		USER("      continue          continue execution\n");
		USER("      next              execute next line\n");
		USER("      step              execute next line, enter functions\n");
		USER("      return            execute until end of current function\n");
		USER("      break             interrupt execution\n");
		USER("      setpc <location>  set PC to <location>\n");
		USER("      goto <location>   execute until <location>\n");
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
			case SETPC:
			case GOTO:
				USER("usage %s %s <location>\n", argv[0], argv[i]);
				USER("   <location> can be any of the following\n");
				USER("      - line number in the current source file\n");
				USER("      - [+-]offset\n");
				USER("      - filename:line number\n");
				USER("      - filename:function\n");
				USER("      - function\n");
				USER("      - function:label\n");
				USER("      - label\n");
				USER("      - *address\n");
				USER("\n");
				break;

			default:
				USER("no further help available for sub-command \"%s\"\n", argv[i]);
			};
		}
	}

	ui->win_cursor_set(ui->win_getid(USERLOG_NAME), -1);
	ui->atomic(false);
}

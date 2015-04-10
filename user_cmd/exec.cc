#include <common/log.h>
#include <common/file.h>
#include <gdb/gdb.h>
#include <gdb/location.h>
#include <gui/gui.h>
#include <user_cmd/cmd.h>
#include <user_cmd/subcmd.hash.h>


/* global functions */
int cmd_exec_exec(gdbif* gdb, int argc, char** argv){
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

			if(gdb->mi_issue_cmd((char*)"file-list-exec-source-file", RC_DONE, result_to_location, (void**)&loc, "") != 0)
				return 0;

			if(FILE_EXISTS(loc->fullname)){
				ui->win_anno_add(ui->win_create(loc->fullname), loc->line, "ip", "White", "Black");
				ui->win_cursor_set(ui->win_create(loc->fullname), loc->line);
			}
			else
				USER("file \"%s\" does not exist\n", loc->fullname);

			delete loc;
		}
		else{
			USER("error executing \"%s\" - inferior is running\n", argv[1]);
			return 0;
		}
	}
	else{
		if(gdb->mi_issue_cmd((char*)"file-list-exec-source-file", RC_DONE, result_to_location, (void**)&loc, "") != 0)
			return 0;

		if(FILE_EXISTS(loc->fullname))	ui->win_anno_delete(ui->win_create(loc->fullname), loc->line, "ip");
		else							USER("file \"%s\" does not exist\n", loc->fullname);

		delete loc;

		if(scmd->id == RUN)				r = gdb->mi_issue_cmd((char*)"exec-run", (gdb_result_class_t)(RC_DONE | RC_RUNNING), 0, 0, "");
		else if(scmd->id == NEXT)		r = gdb->mi_issue_cmd((char*)"exec-next", (gdb_result_class_t)(RC_DONE | RC_RUNNING), 0, 0, "");
		else if(scmd->id == STEP)		r = gdb->mi_issue_cmd((char*)"exec-step", (gdb_result_class_t)(RC_DONE | RC_RUNNING), 0, 0, "");
		else if(scmd->id == RETURN)		r = gdb->mi_issue_cmd((char*)"exec-finish", (gdb_result_class_t)(RC_DONE | RC_RUNNING), 0, 0, "");
		else if(scmd->id == GOTO)		r = gdb->mi_issue_cmd((char*)"exec-until", (gdb_result_class_t)(RC_DONE | RC_RUNNING), 0, 0, "%ss %d", argv + 2, argc - 2);
		else if(scmd->id == JUMP){
			if(gdb->mi_issue_cmd((char*)"break-insert", RC_DONE, 0, 0, "-t %ss %d", argv + 2, argc - 2) != 0)
				return 0;
				
			r = gdb->mi_issue_cmd((char*)"exec-jump", (gdb_result_class_t)(RC_DONE | RC_RUNNING), 0, 0, "%ss %d", argv + 2, argc - 2);
		}
		else if(scmd->id == CONTINUE){
			r = gdb->mi_issue_cmd((char*)"exec-continue", (gdb_result_class_t)(RC_DONE | RC_RUNNING), 0, 0, "");

			if(r != 0){
				USER("error executing \"%s\", trying to start first inferior\n", argv[1]);
				r = gdb->mi_issue_cmd((char*)"exec-run", (gdb_result_class_t)(RC_DONE | RC_RUNNING), 0, 0, "");
			}
		}
		else if(scmd->id == BREAK){
			USER("error executing \"%s\" - inferior is not running\n", argv[1]);
			return 0;
		}

		if(r != 0)
			return 0;
	}

	USER("%s\n", argv[1]);

	return 0;
}

void cmd_exec_help(int argc, char** argv){
	unsigned int i;
	const struct user_subcmd_t* scmd;


	if(argc == 1){
		USER("usage: %s [sub-command] <args>...\n", argv[0]);
		USER("   sub-commands:\n");
		USER("      run              (re)start inferior execution\n");
		USER("      continue         continue execution\n");
		USER("      next             execute next line\n");
		USER("      step             execute next line, enter functions\n");
		USER("      return           execute until end of current function\n");
		USER("      break            interrupt execution\n");
		USER("      jump <location>  set PC to <location>\n");
		USER("      goto <location>  execute until <location>\n");
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
			case JUMP:
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

}

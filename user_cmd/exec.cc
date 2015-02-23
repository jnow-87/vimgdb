#include <common/log.h>
#include <gdb/gdb.h>
#include <gdb/result.h>
#include <gdb/arglist.h>
#include <gui/gui.h>
#include <user_cmd/cmd.h>
#include <user_cmd/subcmd.hash.h>


/* global functions */
int cmd_exec_exec(gdbif* gdb, int argc, char** argv){
	const char* cmd_str;
	const struct user_subcmd_t* scmd;
	response_t* resp;
	arglist_t* param, *el;


	if(argc < 2){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_exec_help(1, argv);
		return 0;
	}

	param = 0;
	scmd = user_subcmd::lookup(argv[1], strlen(argv[1]));

	if(scmd == 0){
		USER("invalid sub-command \"%s\" to command \"%s\"\n", argv[1], argv[0]);
		return 0;
	}

	if(scmd->id == RUN)				resp = gdb->mi_issue_cmd((char*)"exec-run", 0, param);
	else if(scmd->id == CONTINUE)	resp = gdb->mi_issue_cmd((char*)"exec-continue", 0, param);
	else if(scmd->id == NEXT)		resp = gdb->mi_issue_cmd((char*)"exec-next", 0, param);
	else if(scmd->id == STEP)		resp = gdb->mi_issue_cmd((char*)"exec-step", 0, param);
	else if(scmd->id == RETURN)		resp = gdb->mi_issue_cmd((char*)"exec-finish", 0, param);
	else if(scmd->id == JUMP){
		arg_add_string(param, argv[2], false);
		resp = gdb->mi_issue_cmd((char*)"exec-jump", 0, param);
	}
	else if(scmd->id == GOTO){
		arg_add_string(param, argv[2], false);
		resp = gdb->mi_issue_cmd((char*)"exec-until", 0, param);
	}
	else if(scmd->id == BREAK){
		return gdb->sigsend(SIGINT);
	}

	if(resp == 0){
		WARN("error issuing mi command\n");
		arg_clear(param);
		return -1;
	}

	switch(resp->rclass){
	case RC_DONE:
	case RC_RUNNING:
		USER("done: exec \"%s %s\"\n", argv[0], argv[1]);
		break;

	case RC_ERROR:
		USER("gdb reported error for command \"%s %s\"\n\t%s\n", argv[0], argv[1], resp->result->value->value);
		break;

	default:
		WARN("unhandled result class %d for \"%s %s\"\n", resp->rclass, argv[0], argv[1]);
		break;
	};

	gdb_result_free(resp->result);
	arg_clear(param);
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

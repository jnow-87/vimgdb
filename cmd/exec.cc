#include <common/log.h>
#include <common/list.h>
#include <gdb/gdb.h>
#include <gdb/result.h>
#include <gui/gui.h>
#include <cmd/cmd.h>
#include <cmd/subcmd.hash.h>


/* global functions */
int cmd_exec_exec(gdbif* gdb, int argc, char** argv){
	const char* cmd_str;
	const struct subcmd_t* scmd;


	if(argc < 2){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_exec_help(1, argv);
		return -1;
	}

	scmd = subcmd::lookup(argv[1], strlen(argv[1]));
	if(scmd == 0){
		USER("invalid sub-command \"%s\" to command \"%s\"\n", argv[1], argv[0]);
		return -1;
	}

	switch(scmd->id){
	case RUN:
		cmd_str = "exec-run";
		break;

	case CONTINUE:
		cmd_str = "exec-continue";
		break;

	case NEXT:
		cmd_str = "exec-next";
		break;

	case STEP:
		cmd_str = "exec-step";
		break;

	case RETURN:
		cmd_str = "exec-finish";
		break;

	case BREAK:
		return gdb->sigsend(SIGINT);
		break;

	case JUMP:
		cmd_str = "exec-jump";
		break;

	case GOTO:
		cmd_str = "exec-until";
		break;
	};

	if(gdb->mi_issue_cmd((char*)cmd_str, 0, 0, argv + 2, argc - 2, cmd_exec_resp) < 0){
		WARN("error sending mi command\n");
		return -1;
	}

	return 0;
}

int cmd_exec_resp(result_class_t rclass, result_t* result, char* cmdline, void* data){
	switch(rclass){
	case RC_DONE:
	case RC_RUNNING:
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

void cmd_exec_help(int argc, char** argv){
	unsigned int i;
	const struct subcmd_t* scmd;


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
			scmd = subcmd::lookup(argv[i], strlen(argv[i]));

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

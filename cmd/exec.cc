#include <common/log.h>
#include <common/list.h>
#include <gdb/gdb.h>
#include <gdb/result.h>
#include <gui/gui.h>
#include <cmd/cmd.h>
#include <cmd/subcmd.hash.h>


/* global functions */
int cmd_exec_exec(gdb_if* gdb, int argc, char** argv){
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
	TODO("not yet implemented\n");
	return 0;
}

void cmd_exec_help(int argc, char** argv){
	TODO("not yet implemented\n");
}

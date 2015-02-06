#include <common/log.h>
#include <gdb/gdb.h>
#include <gdb/result.h>
#include <cmd/cmd.h>
#include <cmd/subcmd.hash.h>


int cmd_file_exec(gdb_if* gdb, int argc, char** argv){
	unsigned int i;
	const char* cmd_str;
	const struct subcmd_t* scmd;


	if(argc < 2){
		WARN("too few arguments to command \"%s\"\n", argv[0]);
		return -1;
	}

	if(argc > 3){
		WARN("too many arguments to command \"%s\", at most 2 expected\n", argv[0]);
		return -1;
	}

	cmd_str = "file-exec-and-symbols";

	for(i=1; i<argc-1; i++){
		scmd = subcmd::lookup(argv[i], strlen(argv[i]));
		if(scmd == 0){
			WARN("invalid sub command \"%s\" to \"%s\"\n", argv[i], argv[0]);
			return -1;
		}

		switch(scmd->id){
		case BIN:
			cmd_str = "file-exec-file";
			break;

		case SYM:
			cmd_str = "file-symbol-file";
			break;
		};
	}

	if(gdb->mi_issue_cmd((char*)cmd_str, 0, 0, argv + i, 1, cmd_file_resp) < 0){
		WARN("error sending mi command\n");
		return -1;
	}

	return 0;
}

int cmd_file_resp(result_class_t rclass, result_t* result, char* cmdline){
	switch(rclass){
	case RC_DONE:
		USER("exec of command \"%s\" done\n", cmdline);
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

void cmd_file_help(char* cmd){
	USER("usage: %s [sub-command] <file>\n", cmd);
	USER("   sub-commands:\n");
	USER("      bin   load only code from <file>\n");
	USER("      sym   load debug symbols from <file>\n");
}

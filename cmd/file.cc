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

	// TODO add response handler
	if(gdb->mi_cmd_issue((char*)cmd_str, 0, 0, argv + i, 1, 0) < 0)
		return -1;
	return 0;
}

int cmd_file_resp(int result_class, result_t* result){
	return 0;
}

void cmd_file_help(char* cmd){
	DEBUG("usage: %s [sub-command] <file>\n", cmd);
	DEBUG("   sub-commands:\n");
	DEBUG("      bin   load only code from <file>\n");
	DEBUG("      sym   load debug symbols from <file>\n");
}
#include <common/log.h>
#include <gdb/gdb.h>
#include <gdb/result.h>
#include <cmd/cmd.h>


int cmd_test_exec(gdb_if* gdb, int argc, char** argv){
	unsigned int i;


	USER("command: %s\n", argv[0]);
	USER("arguments:");

	for(i=1; i<argc; i++)
		USER(" (%d, \"%s\")", i, argv[i]);
	USER("\n");

	return 0;
}

void cmd_test_help(char* cmd){
	USER("usage: %s [<arg>...]\n", cmd);
	USER("   test prints all arguments to demonstrate the cmd interface functionality\n");
}

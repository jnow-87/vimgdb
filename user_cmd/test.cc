#include <common/log.h>
#include <gdb/gdb.h>
#include <gdb/result.h>
#include <user_cmd/cmd.h>


int cmd_test_exec(int argc, char** argv){
	unsigned int i;


	USER("command: %s\n", argv[0]);
	USER("arguments:");

	for(i=1; i<argc; i++)
		USER(" (%d, \"%s\")", i, argv[i]);
	USER("\n");

	return 0;
}

void cmd_test_help(int argc, char** argv){
	USER("usage: %s [<arg>...]\n", argv[0]);
	USER("   test prints all arguments to demonstrate the user_cmd interface functionality\n");
	USER("\n");
}

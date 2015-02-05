#include <common/log.h>
#include <gdb/gdb.h>
#include <gdb/result.h>
#include <cmd/cmd.h>


int cmd_test_exec(gdb_if* gdb, int argc, char** argv){
	unsigned int i;


	DEBUG("command: %s\n", argv[0]);
	DEBUG("arguments:");

	for(i=1; i<argc; i++)
		DEBUG(" (%d, \"%s\")", i, argv[i]);
	DEBUG("\n");

	return 0;
}

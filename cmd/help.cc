#include <common/log.h>
#include <gdb/gdb.h>
#include <gdb/result.h>
#include <cmd/cmd.h>
#include <cmd/cmd.hash.h>


int cmd_help_exec(gdb_if* gdb, int argc, char** argv){
	unsigned int i;


	DEBUG("user commands:\n");

	for(i=cmd::MIN_HASH_VALUE; i<=cmd::MAX_HASH_VALUE; i++)
		DEBUG("    %7.7s   %s\n", cmd::wordlist[i].name, cmd::wordlist[i].help_msg);
	return 0;
}

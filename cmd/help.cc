#include <common/log.h>
#include <gdb/gdb.h>
#include <gdb/result.h>
#include <cmd/cmd.h>
#include <cmd/cmd.hash.h>


int cmd_help_exec(gdb_if* gdb, int argc, char** argv){
	unsigned int i;
	const struct cmd_t* c;


	if(argc == 1){
		DEBUG("user commands:\n");

		for(i=cmd::MIN_HASH_VALUE; i<=cmd::MAX_HASH_VALUE; i++)
			DEBUG("    %7.7s   %s\n", cmd::wordlist[i].name, cmd::wordlist[i].help_msg);
		DEBUG("\n");
	}
	else{
		for(i=1; i<argc; i++){
			if(strlen(argv[i]) == 0)
				continue;

			c = cmd::lookup(argv[i], strlen(argv[i]));

			if(c == 0){
				DEBUG("invalid command \"%s\"\n", argv[i]);
				continue;
			}

			if(c->help != 0)	c->help(argv[i]);
			else				DEBUG("no further help available for command \"%s\"\n", argv[i]);
			DEBUG("\n");
		}
	}

	return 0;
}

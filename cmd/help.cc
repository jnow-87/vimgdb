#include <common/log.h>
#include <gdb/gdb.h>
#include <gdb/result.h>
#include <cmd/cmd.h>
#include <cmd/cmd.hash.h>


int cmd_help_exec(gdbif* gdb, int argc, char** argv){
	unsigned int i;
	const struct cmd_t* c;


	if(argc == 1){
		USER("user commands:\n");

		for(i=cmd::MIN_HASH_VALUE; i<=cmd::MAX_HASH_VALUE; i++){
			if(cmd::wordlist[i].name[0] != 0)
				USER("    %15.15s   %s\n", cmd::wordlist[i].name, cmd::wordlist[i].help_msg);
		}
		USER("\n");
	}
	else{
		if(strlen(argv[1]) == 0)
			return -1;

		c = cmd::lookup(argv[1], strlen(argv[1]));

		if(c == 0){
			USER("invalid command \"%s\"\n", argv[1]);
			return -1;
		}

		if(c->help != 0)	c->help(argc - 1, argv + 1);
		else				USER("no further help available for command \"%s\"\n", argv[1]);
	}

	return 0;
}

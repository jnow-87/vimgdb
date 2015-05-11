#include <common/defaults.h>
#include <common/log.h>
#include <gui/gui.h>
#include <gdb/gdb.h>
#include <gdb/result.h>
#include <user_cmd/cmd.h>
#include <user_cmd/cmd.hash.h>


int cmd_help_exec(int argc, char** argv){
	unsigned int i;
	const struct user_cmd_t* c;


	if(argc == 1){
		ui->atomic(true);

		USER("user commands:\n");

		for(i=user_cmd::MIN_HASH_VALUE; i<=user_cmd::MAX_HASH_VALUE; i++){
			if(user_cmd::wordlist[i].name[0] != 0)
				USER("    %15.15s   %s\n", user_cmd::wordlist[i].name, user_cmd::wordlist[i].help_msg);
		}

		USER("\n");
		ui->win_cursor_set(ui->win_getid(USERLOG_NAME), -1);

		ui->atomic(false);
	}
	else{
		if(strlen(argv[1]) == 0)
			return 0;

		c = user_cmd::lookup(argv[1], strlen(argv[1]));

		if(c == 0){
			USER("invalid command \"%s\"\n", argv[1]);
			return 0;
		}

		if(c->help != 0)	c->help(argc - 1, argv + 1);
		else				USER("no further help available for command \"%s\"\n", argv[1]);
	}

	return 0;
}

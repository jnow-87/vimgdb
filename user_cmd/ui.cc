#include <common/defaults.h>
#include <common/log.h>
#include <gui/gui.h>
#include <gdb/gdb.h>
#include <gdb/result.h>
#include <user_cmd/cmd.h>
#include <string.h>


int cmd_ui_exec(int argc, char** argv){
	if(argc < 4){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_ui_help(1, argv);
		return 0;
	}

	if(strcmp(argv[2], "ro") == 0){
		ui->win_readonly(ui->win_getid(argv[1]), atoi(argv[3]));
	}
	else
		USER("invalid parameter \"%s\"\n", argv[2]);

	return 0;
}

void cmd_ui_help(int argc, char** argv){
	ui->atomic(true);

	USER("usage: %s <buffer> <param> <value>\n", argv[0]);
	USER("   set buffer parameter <param> to the specified <value>\n\n");
	USER("   <param>\n");
	USER("      ro = [0 | 1]    set the buffer's readonly state\n");
	USER("\n");

	ui->win_cursor_set(ui->win_getid(USERLOG_NAME), -1);
	ui->atomic(false);
}

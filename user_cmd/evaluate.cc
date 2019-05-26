#include <common/defaults.h>
#include <common/log.h>
#include <gui/gui.h>
#include <gdb/gdb.h>
#include <gdb/result.h>
#include <gdb/strlist.h>
#include <user_cmd/cmd.h>


bool cmd_evaluate_exec(int argc, char **argv){
	int i;
	gdb_strlist_t *s;


	if(argc < 2){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_evaluate_help(1, argv);
		return false;
	}

	if(gdb->mi_issue_cmd("data-evaluate-expression", (gdb_result_t**)&s, "\"%ss %d\"", argv + 1, argc - 1) != 0)
		return false;

	ui->win_atomic(0, true);

	for(i=1; i<argc; i++)
		USER("%s ", argv[i]);

	USER("= %s\n", s->s);
	ui->win_cursor_set(ui->win_getid(USERLOG_NAME), -1);

	ui->win_atomic(0, false);

	delete s;

	return false;
}

void cmd_evaluate_help(int argc, char **argv){
	ui->win_atomic(0, true);

	USER("usage: %s [<expression> ]\n", argv[0]);
	USER("   evaluate expression <expression>, which is allowed to contain spaces\n");

	ui->win_cursor_set(ui->win_getid(USERLOG_NAME), -1);
	ui->win_atomic(0, false);
}

#include <common/defaults.h>
#include <common/log.h>
#include <gui/gui.h>
#include <gdb/gdb.h>
#include <gdb/result.h>
#include <user_cmd/cmd.h>


int cmd_evaluate_exec(int argc, char** argv){
	int i;
	gdb_result_t* r;


	if(argc < 2){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_evaluate_help(1, argv);
		return 0;
	}

	r = 0;
	if(gdb->mi_issue_cmd((char*)"data-evaluate-expression", RC_DONE, 0, (void**)&r, "\"%ss %d\"", argv + 1, argc - 1) != 0)
		return -1;

	ui->atomic(true);

	for(i=1; i<argc; i++)
		USER("%s ", argv[i]);

	USER("= %s\n", r->value->value);
	ui->win_cursor_set(ui->win_getid(USERLOG_NAME), -1);

	ui->atomic(false);


	gdb_result_free(r);
	return 0;
}

void cmd_evaluate_help(int argc, char** argv){
	ui->atomic(true);

	USER("usage: %s [<expression> ]\n", argv[0]);
	USER("   evaluate expression <expression>, which is allowed to contain spaces\n");

	ui->win_cursor_set(ui->win_getid(USERLOG_NAME), -1);
	ui->atomic(false);
}

#include <common/defaults.h>
#include <common/map.h>
#include <common/log.h>
#include <common/list.h>
#include <common/dynarray.h>
#include <gui/gui.h>
#include <gdb/gdb.h>
#include <gdb/variable.h>
#include <user_cmd/cmd.h>
#include <user_cmd/subcmd.hash.h>
#include <stdlib.h>
#include <map>
#include <string>


using namespace std;


/* static variables */
static map<unsigned int, gdb_variable_t*> line_map;


/* global functions */
int cmd_var_exec(int argc, char **argv){
	gdb_variable_t *var;
	const struct user_subcmd_t *scmd;
	FILE *fp;
	map<unsigned int, gdb_variable_t*>::iterator lit;
	map<string, gdb_variable_t*>::iterator sit;


	if(argc < 2){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_var_help(1, argv);
		return 0;
	}

	scmd = user_subcmd::lookup(argv[1], strlen(argv[1]));

	if(scmd == 0){
		USER("invalid sub-command \"%s\" to command \"%s\"\n", argv[1], argv[0]);
		return 0;
	}

	if(((scmd->id == ADD || scmd->id == DELETE || scmd->id == FOLD || scmd->id == COMPLETE || scmd->id == EXPORT) && argc < 3) || ((scmd->id == SET || scmd->id == FORMAT) && argc < 4)){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_var_help(2, argv);
		return 0;
	}

	switch(scmd->id){
	case DELETE:
	case FOLD:
	case SET:
	case FORMAT:
		var = MAP_LOOKUP(line_map, atoi(argv[2]));

		if(var == 0){
			USER("no variable at line %s\n", argv[2]);
			return 0;
		}

		break;

	default:
		break;
	};

	switch(scmd->id){
	case ADD:
		var = gdb_variable_t::acquire(argv[2], O_USER);

		if(var == 0)
			return -1;

		if(var->refcnt == 1){
			USER("add variable \"%s\" for expression \"%s\"\n", var->name, var->exp);
			cmd_var_print();
		}
		else
			// never reference a user-variable more than once
			var->refcnt = 1;

		break;
	
	case DELETE:
		while(var->parent != 0)
			var = var->parent;

		USER("delete variable \"%s\"\n", var->name);

		if(gdb_variable_t::release(var) != 0)
			return -1;

		cmd_var_print();
		break;

	case FOLD:
		if(var->nchilds){
			var->init_childs();

			if(var->childs_visible)	var->childs_visible = false;
			else					var->childs_visible = true;
		}
		else if(var->parent)
			var->parent->childs_visible = false;

		cmd_var_print();
		break;

	case SET:
		var->set(argc - 3, argv + 3);
		gdb->memory_update();
		break;

	case FORMAT:
		var->format(argv[3]);
		cmd_var_print();
		break;

	case COMPLETE:
		fp = fopen(argv[2], "w");

		if(fp == 0)
			return -1;

		for(lit=line_map.begin(); lit!=line_map.end(); lit++)
			fprintf(fp, "%d\\n", lit->first);

		fclose(fp);
		break;

	case EXPORT:
		fp = fopen(argv[2], "w");

		if(fp == 0){
			DEBUG("unable to open file \"%s\"\n", argv[2]);
			return 0;
		}

		for(sit=gdb_user_var.begin(); sit!=gdb_user_var.end(); sit++)
			fprintf(fp, "Variable add %s\n", sit->second->exp);

		fprintf(fp, "\n");
		fclose(fp);

		USER("export variables to \"%s\"\n", argv[2]);
		break;

	case VIEW:
		cmd_var_print();
		break;

	default:
		USER("unhandled sub command \"%s\" to \"%s\"\n", argv[1], argv[0]);
	};

	return 0;
}

void cmd_var_cleanup(){
	line_map.clear();
}

void cmd_var_help(int argc, char **argv){
	int i;
	const struct user_subcmd_t *scmd;


	ui->win_atomic(0, true);

	if(argc == 1){
		USER("usage: %s [sub-command] <args>...\n", argv[0]);
		USER("   sub-commands:\n");
		USER("      add <expr>               add variable for expression <expr>\n");
		USER("      delete <line>            delete variable\n");
		USER("      fold <line>              fold/unfold variable\n");
		USER("      format <line> <fmt>      change variable output format\n");
		USER("      set <line> <value>       set variable\n");
		USER("      complete <file> <sync>   get list of variables\n");
		USER("      export <file> <sync>     export user variables to vim script\n");
		USER("      view                     update variable window\n");
		USER("\n");
	}
	else{
		for(i=1; i<argc; i++){
			scmd = user_subcmd::lookup(argv[i], strlen(argv[i]));

			if(scmd == 0){
				USER("invalid sub-command \"%s\" to command \"%s\"\n", argv[i], argv[0]);
				continue;
			}

			switch(scmd->id){
			case ADD:
				USER("usage %s %s <expr>\n", argv[0], argv[i]);
				USER("   add new variable for expression <expr>, <expr> being any valid c/c++-expression\n");
				USER("\n");
				break;

			case DELETE:
				USER("usage %s %s <line>\n", argv[0], argv[i]);
				USER("   delete variable at line <line>\n");
				USER("\n");
				break;

			case FOLD:
				USER("usage %s %s <line>\n", argv[0], argv[i]);
				USER("   fold variable at line <line>\n");
				USER("\n");
				break;

			case FORMAT:
				USER("usage %s %s <line> <format>\n", argv[0], argv[i]);
				USER("   change format of variable at line <line> to <format>\n");
				USER("   <format> = binary | decimal | hexadecimal | octal | natural\n");
				USER("\n");
				break;

			case SET:
				USER("usage %s %s <line> <value>\n", argv[0], argv[i]);
				USER("   set variable value at line <line>\n");
				USER("\n");
				break;

			case COMPLETE:
				USER("usage %s %s <file> <sync>\n", argv[0], argv[i]);
				USER("   print '\\n' separated list of line numbers that contain variables to file <file>, using file <sync> to sync with vim\n");
				USER("\n");
				break;

			case EXPORT:
				USER("usage %s %s <file>\n", argv[0], argv[1]);
				USER("   export user variables to vim script <file>, using file <sync> to sync with vim\n");
				USER("   note: restore of variables only works for global variables\n");
				USER("\n");
				break;

			case VIEW:
				break;

			default:
				USER("invalid sub-command \"%s\" to command \"%s\"\n", argv[i], argv[0]);
			};
		}
	}

	ui->win_cursor_set(ui->win_getid(USERLOG_NAME), -1);
	ui->win_atomic(0, false);
}

int cmd_var_print(){
	static dynarray obuf;
	int win_id;
	unsigned int line;
	map<string, gdb_variable_t*>::iterator it;


	win_id = ui->win_getid(VARIABLES_NAME);

	if(win_id < 0)
		return 0;

	line_map.clear();
	obuf.clear();

	line = 1;

	for(it=gdb_user_var.begin(); it!=gdb_user_var.end(); it++){
		if(it->second->parent == 0)
			it->second->print(&obuf, &line, &line_map, true);
	}

	ui->win_atomic(win_id, true);

	ui->win_clear(win_id);
	ui->win_print(win_id, obuf.data());

	ui->win_atomic(win_id, false);

	return 0;
}

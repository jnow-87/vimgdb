#include <common/defaults.h>
#include <common/log.h>
#include <common/map.h>
#include <common/list.h>
#include <gui/gui.h>
#include <gdb/gdb.h>
#include <gdb/result.h>
#include <gdb/variable.h>
#include <gdb/strlist.h>
#include <user_cmd/cmd.h>
#include <user_cmd/subcmd.hash.h>
#include <map>
#include <string>


using namespace std;


/* static variables */
static map<unsigned int, gdb_variable_t*> line_map;


/* global functions */
int cmd_register_init(){
	static bool initialised = false;
	gdb_strlist_t *names, *name;
	gdb_variable_t* var;


	if(initialised)
		return 0;

	initialised = true;

	/* get register names */
	if(gdb->mi_issue_cmd("data-list-register-names", (gdb_result_t**)&names, "") != 0)
		return -1;

	ui->atomic(true);

	/* create variables */
	list_for_each(names, name){
		if(name->s[0] == 0)
			continue;

		var = gdb_variable_t::acquire(name->s, O_REGISTER);

		if(var == 0)
			goto err;

		if(var->format("hexadecimal") != 0)
			goto err;
	}

	cmd_register_print();

	ui->atomic(false);
	delete names;

	return 0;

err:
	ui->atomic(false);
	delete names;

	return -1;
}

int cmd_register_exec(int argc, char** argv){
	gdb_variable_t* var;
	const struct user_subcmd_t* scmd;
	FILE* fp;
	map<unsigned int, gdb_variable_t*>::iterator it;


	if(argc < 2){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_register_help(1, argv);
		return 0;
	}

	scmd = user_subcmd::lookup(argv[1], strlen(argv[1]));

	if(scmd == 0){
		USER("invalid sub-command \"%s\" to command \"%s\"\n", argv[1], argv[0]);
		return 0;
	}

	if(((scmd->id == FOLD) && argc < 3) || ((scmd->id == SET || scmd->id == FORMAT || scmd->id == COMPLETE) && argc < 4)){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_register_help(2, argv);
		return 0;
	}

	switch(scmd->id){
	case SET:
	case FOLD:
	case FORMAT:
		var = MAP_LOOKUP(line_map, atoi(argv[2]));

		if(var == 0){
			USER("no register at line \"%s\"\n", argv[2]);
			return 0;
		}
		break;

	default:
		var = 0;
		break;
	};

	switch(scmd->id){
	case SET:
		var->set(argc - 3, argv + 3);

		gdb_variable_t::get_changed();

		cmd_register_print();
		break;

	case FOLD:
		if(var->nchilds){
			var->init_childs();

			if(var->childs_visible)	var->childs_visible = false;
			else					var->childs_visible = true;
		}
		else if(var->parent)
			var->parent->childs_visible = false;

		cmd_register_print();
		break;

	case FORMAT:
		var->format(argv[3]);
		cmd_register_print();
		break;

	case COMPLETE:
		fp = fopen(argv[2], "w");

		if(fp == 0)
			return -1;

		for(it=line_map.begin(); it!=line_map.end(); it++)
			fprintf(fp, "%d\\n", it->first);

		fclose(fp);

		/* signal data availability */
		fp = fopen(argv[3], "w");

		if(fp == 0)
			return -1;

		fprintf(fp, "1\n");
		fclose(fp);
		break;

	case VIEW:
		cmd_register_print();
		break;

	default:
		USER("unhandled sub command \"%s\" to \"%s\"\n", argv[1], argv[0]);
	};

	return 0;
}

void cmd_register_help(int argc, char** argv){
	int i;
	const struct user_subcmd_t* scmd;


	ui->atomic(true);

	if(argc == 1){
		USER("usage: %s [sub-command] <args>...\n", argv[0]);
		USER("   sub-commands:\n");
		USER("      fold <line>          fold/unfold variable/frame\n");
		USER("      format <line> <fmt>  change variable output format\n");
		USER("      set <line> <value>   set variable\n");
		USER("      complete <filename>  get list of variables/frames\n");
		USER("      view                 update register window\n");
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
			case FOLD:
				USER("usage %s %s <line>\n", argv[0], argv[i]);
				USER("   fold variable or frame at line <line>\n");
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
				USER("usage %s %s <filename>\n", argv[0], argv[i]);
				USER("          print '\\n' seprated list of line numbers that contain variables or frames to file <filename>\n");
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
	ui->atomic(false);
}

int cmd_register_print(){
	int win_id;
	unsigned int line;
	map<string, gdb_variable_t*>::iterator it;


	win_id = ui->win_getid(REGISTERS_NAME);

	if(win_id < 0)
		return 0;

	line = 1;
	line_map.clear();

	ui->atomic(true);
	ui->win_clear(win_id);

	for(it=gdb_register_var.begin(); it!=gdb_register_var.end(); it++){
		it->second->print(win_id, &line, &line_map, true);
	}

	ui->atomic(false);

	return 0;
}

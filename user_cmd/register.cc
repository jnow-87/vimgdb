#include <common/log.h>
#include <common/map.h>
#include <gui/gui.h>
#include <gdb/gdb.h>
#include <gdb/result.h>
#include <gdb/variable.h>
#include <user_cmd/cmd.h>
#include <user_cmd/subcmd.hash.h>
#include <map>
#include <string>


using namespace std;


/* static variables */
static map<unsigned int, gdb_variable_t*> line_map;


/* global functions */
int cmd_register_init(){
	gdb_result_t* r;
	gdb_value_t* val;
	gdb_variable_t* var;
	map<string, gdb_variable_t*>::iterator it;


	/* clear register variables if present */
	for(it=gdb_register_var.begin(); it!=gdb_register_var.end(); it++)
		gdb_variable_t::release(it->second);

	gdb_register_var.clear();

	/* get register names */
	if(gdb->mi_issue_cmd((char*)"data-list-register-names", RC_DONE, 0, (void**)&r, "") != 0)
		return -1;

	if(r->var_id != IDV_REG_NAMES)
		goto err_0;

	ui->atomic(true);

	/* create variables */
	list_for_each((gdb_value_t*)r->value->value, val){
		if(((char*)val->value)[0] == 0)
			continue;

		var = gdb_variable_t::acquire((char*)val->value, O_REGISTER);

		if(var == 0)
			goto err_1;

		if(var->format((char*)"hexadecimal") != 0)
			goto err_1;
	}

	cmd_register_print();

	gdb_result_free(r);
	ui->atomic(false);

	return 0;

err_1:
	ui->atomic(false);

err_0:
	gdb_result_free(r);
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

	if(((scmd->id == FOLD || scmd->id == GET) && argc < 3) || ((scmd->id == SET || scmd->id == FORMAT) && argc < 4)){
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

	case GET:
		fp = fopen(argv[2], "w");

		if(fp == 0)
			return -1;

		for(it=line_map.begin(); it!=line_map.end(); it++)
			fprintf(fp, "%d\\n", it->first);

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
	unsigned int i;
	const struct user_subcmd_t* scmd;


	if(argc == 1){
		USER("usage: %s [sub-command] <args>...\n", argv[0]);
		USER("   sub-commands:\n");
		USER("      fold <line>          fold/unfold variable/frame\n");
		USER("      format <line> <fmt>  change variable output format\n");
		USER("      set <line> <value>   set variable\n");
		USER("      get <filename>       get list of variables/frames\n");
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

			case GET:
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
}

int cmd_register_print(){
	int win_id;
	unsigned int line;
	map<string, gdb_variable_t*>::iterator it;


	win_id = ui->win_getid("registers");

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

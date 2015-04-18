#include <common/log.h>
#include <common/list.h>
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


/* local prototypes */
void var_print();
void var_print(gdb_variable_t* var, int* line, int win_id, int rec_lvl);


/* global functions */
int cmd_var_exec(int argc, char** argv){
	gdb_variable_t *var, *c;
	const struct user_subcmd_t* scmd;
	FILE* fp;
	map<unsigned int, gdb_variable_t*>::iterator it;


	if(argc < 2){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_var_help(1, argv);
		return 0;
	}

	var = 0;
	scmd = user_subcmd::lookup(argv[1], strlen(argv[1]));

	if(scmd == 0){
		USER("invalid sub-command \"%s\" to command \"%s\"\n", argv[1], argv[0]);
		return 0;
	}

	if((scmd->id == ADD || scmd->id == DELETE || scmd->id == FOLD || scmd->id == SET || scmd->id == GET) && argc < 3){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_var_help(2, argv);
		return 0;
	}

	switch(scmd->id){
	case ADD:
		var = new gdb_variable_t();
		if(var->create(argv[2], O_USER) == 0)
			USER("add variable \"%s\" for expression \"%s\"\n", var->name, var->exp);

		var_print();
		break;
	
	case DELETE:
		it = line_map.find(atoi(argv[2]));

		if(it == line_map.end()){
			USER("no variable at line %s\n", argv[2]);
			return -1;
		}

		var = it->second;

		while(var->parent != 0)
			var = var->parent;

		if(var->destroy() == 0){
			USER("delete variable \"%s\"\n", var->name);
			delete var;
		}

		var_print();
		break;

	case FOLD:
		it = line_map.find(atoi(argv[2]));

		if(it == line_map.end()){
			USER("no variable at line \"%s\"\n", argv[2]);
			return -1;
		}

		var = it->second;

		if(var->nchilds){
			var->init_childs();

			if(var->childs_visible)	var->childs_visible = false;
			else					var->childs_visible = true;
		}
		else if(var->parent)
			var->parent->childs_visible = false;

		var_print();
		break;

	case SET:
		it = line_map.find(atoi(argv[2]));

		if(it == line_map.end()){
			USER("no variable at line \"%s\"\n", argv[2]);
			return -1;
		}

		var = it->second;
		var->set(argc - 3, argv + 3);
		var_print();
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
		cmd_var_update();
		break;

	default:
		USER("unhandled sub command \"%s\" to \"%s\"\n", argv[1], argv[0]);
	};

	return 0;
}

void cmd_var_help(int argc, char** argv){
	unsigned int i;
	const struct user_subcmd_t* scmd;


	if(argc == 1){
		USER("usage: %s [sub-command] <args>...\n", argv[0]);
		USER("   sub-commands:\n");
		USER("      add <expr>              add variable for expression <expr>\n");
		USER("      delete <var-def>        delete variable\n");
		USER("      fold <var-def>          fold/unfold variable\n");
		USER("      set <var-def> <value>   set variable\n");
		USER("      get <filename>          get list of variables\n");
		USER("      view                    update variable window\n");
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
				USER("usage %s %s <var-def>\n", argv[0], argv[i]);
				USER("   delete variable <var-def>, <var-def> being either the variable name or the line in the variable window\n");
				USER("\n");
				break;

			case FOLD:
				USER("usage %s %s <var-def>\n", argv[0], argv[i]);
				USER("   fold variable <var-def>, <var-def> being either the variable name or the line in the variable window\n");
				USER("\n");
				break;

			case SET:
				USER("usage %s %s <var-def> <value>\n", argv[0], argv[i]);
				USER("   set variable <var-def> to value <value>, <var-def> being either the variable name or the line in the variable window\n");
				USER("\n");
				break;

			case GET:
				USER("usage %s %s <filename>\n", argv[0], argv[i]);
				USER("          print '\\n' seprated list of variables to file <filename>\n");
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

int cmd_var_update(){
	if(ui->win_getid("variables") < 0)
		return 0;

	gdb_variables_update();
	var_print();

	return 0;
}

/* local functions */
void var_print(){
	int win_id_var, i;
	map<string, gdb_variable_t*>::iterator it;


	win_id_var = ui->win_getid("variables");

	if(win_id_var < 0)
		return;

	ui->atomic(true);
	ui->win_clear(win_id_var);
	line_map.clear();

	i = 1;

	for(it=gdb_var_lst.begin(); it!=gdb_var_lst.end(); it++){
		if(it->second->parent == 0)
			var_print(it->second, &i, win_id_var, 1);
	}

	ui->atomic(false);
}

void var_print(gdb_variable_t* var, int* line, int win_id, int rec_lvl){
	char rec_s[rec_lvl + 1];
	gdb_variable_t* v;
	map<unsigned int, gdb_variable_t*>::iterator it;


	/* assemble blank string */
	memset(rec_s, ' ', rec_lvl);
	rec_s[rec_lvl] = 0;

	/* update variable value */
	var->update();

	/* update UI */
	ui->win_print(win_id, "%s%s%s %s = %s\n", (var->modified ? "*" : " "), rec_s, (var->nchilds == 0 ? "   " : (var->childs_visible ? "[-]" : "[+]")), var->exp, var->value);

	/* update variable structs */
	var->modified = false;
	line_map[*line] = var;
	(*line)++;

	/* print childs */
	if(var->childs_visible){
		list_for_each(var->childs, v)
			var_print(v, line, win_id, rec_lvl + 1);
	}

	if(rec_lvl == 1){
		ui->win_print(win_id, "\n");
		(*line)++;
	}
}

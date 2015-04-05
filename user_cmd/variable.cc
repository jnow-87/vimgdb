#include <common/log.h>
#include <common/list.h>
#include <gui/gui.h>
#include <gdb/gdb.h>
#include <gdb/variable.h>
#include <gdb/variablelist.h>
#include <user_cmd/cmd.h>
#include <user_cmd/subcmd.hash.h>
#include <stdlib.h>
#include <map>
#include <string>


using namespace std;


/* static variables */
static gdb_variablelist var_lst;


/* global functions */
int cmd_var_exec(gdbif* gdb, int argc, char** argv){
	gdb_variable_t *var, *c;
	const struct user_subcmd_t* scmd;


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

	switch(scmd->id){
	case ADD:
		if(gdb->mi_issue_cmd((char*)"var-create", RC_DONE, result_to_variable, (void**)&var, "- * %s", argv[2]) == 0){
			if(gdb->mi_issue_cmd((char*)"var-info-expression", RC_DONE, result_to_variable, (void**)&var, "%s", var->name) == 0){
				USER("add variable \"%s\" for expression \"%s\"\n", var->name, var->exp);
				var_lst.add(var->name, var);
			}
		}
		break;
	
	case DELETE:
		var = var_lst.find(argv[2]);

		if(var == 0){
			USER("no variable at line %s\n", argv[2]);
			return -1;
		}

		while(var->parent != 0)
			var = var->parent;

		if(gdb->mi_issue_cmd((char*)"var-delete", RC_DONE, 0, 0, "%s", var->name) == 0){
			USER("delete variable \"%s\"\n", var->name);

			var_lst.rm(var);
			delete var;
		}
		break;

	case FOLD:
		var = var_lst.find(argv[2]);

		if(var == 0){
			USER("no variable for expression \"%s\"\n", argv[2]);
			return -1;
		}

		if(var->nchilds){
			if(var->childs == 0){
				gdb->mi_issue_cmd((char*)"var-list-children", RC_DONE, result_to_variable, (void**)&var, "%s", var->name);

				list_for_each(var->childs, c){
					gdb->mi_issue_cmd((char*)"var-evaluate-expression", RC_DONE, result_to_variable, (void**)&c, "%s", c->name);
					var_lst.add(c->name, c);
				}
			}

			if(var->childs_visible)	var->childs_visible = false;
			else					var->childs_visible = true;
		}
		else if(var->parent)
			var->parent->childs_visible = false;
		break;

	case SET:
		var = var_lst.find(argv[2]);

		if(var == 0){
			USER("no variable for expression \"%s\"\n", argv[2]);
			return -1;
		}

		gdb->mi_issue_cmd((char*)"var-assign", RC_DONE, 0, 0, "%s \"%ss %d\"", var->name, argv + 3, argc - 3);
		var->modified = true;
		break;

	default:
		USER("unhandled sub command \"%s\" to \"%s\"\n", argv[1], argv[0]);
	};

	var_lst.print(gdb);

	return 0;
}

void cmd_var_help(int argc, char** argv){
	// TODO add
	TODO("not yet implemented\n");
}

int cmd_var_update(gdbif* gdb){
	var_lst.update(gdb);
	var_lst.print(gdb);

	return 0;
}

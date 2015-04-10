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

	if((scmd->id == ADD || scmd->id == DELETE || scmd->id == FOLD || scmd->id == SET || scmd->id == GET) && argc < 3){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_var_help(2, argv);
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

		var_lst.print(gdb);
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

		var_lst.print(gdb);
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

		var_lst.print(gdb);
		break;

	case SET:
		var = var_lst.find(argv[2]);

		if(var == 0){
			USER("no variable for expression \"%s\"\n", argv[2]);
			return -1;
		}

		gdb->mi_issue_cmd((char*)"var-assign", RC_DONE, 0, 0, "%s \"%ss %d\"", var->name, argv + 3, argc - 3);
		var->modified = true;
		var_lst.print(gdb);
		break;

	case GET:
		var_lst.get_list(argv[2]);
		break;

	case VIEW:
		var_lst.update(gdb);
		var_lst.print(gdb);
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

int cmd_var_update(gdbif* gdb){
	var_lst.update(gdb);
	var_lst.print(gdb);

	return 0;
}

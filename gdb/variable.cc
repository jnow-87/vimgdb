#include <common/map.h>
#include <common/list.h>
#include <common/log.h>
#include <gui/gui.h>
#include <gdb/gdb.h>
#include <gdb/variable.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <string>


using namespace std;


/* global variables */
map<string, gdb_variable_t*> gdb_var_lst;


/* class */
gdb_variable_t::gdb_variable_t(){
	name = 0;
	exp = 0;
	type = 0;
	value = 0;
	parent = 0;
	childs = 0;
	modified = true;
	childs_visible = false;
	inscope = true;
	argument = false;
	nchilds = 0;
	origin = O_UNKNOWN;
	refcnt = 1;
}

gdb_variable_t::~gdb_variable_t(){
	gdb_variable_t* c;
	map<string, gdb_variable_t*>::iterator it;


	it = gdb_var_lst.find(name);

	if(it != gdb_var_lst.end())
		gdb_var_lst.erase(it);

	delete name;
	delete exp;
	delete type;
	delete value;

	list_for_each(childs, c){
		list_rm(&childs, c);
		delete c;
	}
}

gdb_variable_t* gdb_variable_t::acquire(char* expr, char* context, unsigned int frame){
	gdb_variable_t* v;
	string key;


	if(context){
		key = "s:";
		key += context;
		key += ":";
	}
	else
		key = "u:";

	key += expr;

	v = MAP_LOOKUP(gdb_var_lst, key);

	if(v != 0){
		v->refcnt++;
		return v;
	}

	v = new gdb_variable_t;

	if(gdb->mi_issue_cmd((char*)"var-create", RC_DONE, gdb_variable_t::result_to_variable, (void**)&v, "--thread %u --frame %u \"%s\" %s %s", gdb->threadid(), frame, key.c_str(), (context ? "*" : "@"), expr) == 0){
		v->origin = context ? O_STACK : O_USER;

		gdb_var_lst[key] = v;

		if(gdb->mi_issue_cmd((char*)"var-info-expression", RC_DONE, gdb_variable_t::result_to_variable, (void**)&v, "\"%s\"", v->name) == 0)
			return v;
	}

	return 0;
}

int gdb_variable_t::release(gdb_variable_t* v){
	map<string, gdb_variable_t*>::iterator it;


	if(--v->refcnt == 0){
		it = gdb_var_lst.find(v->name);

		if(it != gdb_var_lst.end()){
			gdb_var_lst.erase(it);

			if(gdb->mi_issue_cmd((char*)"var-delete", RC_DONE, 0, 0, "\"%s\"", v->name) != 0)
				return -1;
		}

		delete v;
	}

	return 0;
}

int gdb_variable_t::get_changed(){
	gdb->mi_issue_cmd((char*)"var-update", RC_DONE, gdb_variable_t::result_to_change_list, 0, "*");
	return 0;
}

int gdb_variable_t::result_to_variable(gdb_result_t* result, void** _var){
	gdb_variable_t *var, *child;
	gdb_result_t *r, *c;


	if(*_var == 0)
		*_var = new gdb_variable_t;

	var = (gdb_variable_t*)*_var;

	list_for_each(result, r){
		switch(r->var_id){
		case IDV_NAME:
			delete var->name;
			var->name = (char*)r->value->value;
			r->value->value = 0;
			break;

		case IDV_TYPE:
			delete var->type;
			var->type = (char*)r->value->value;
			r->value->value = 0;
			break;

		case IDV_VALUE:
			delete var->value;
			var->value = (char*)r->value->value;
			r->value->value = 0;
			break;

		case IDV_NUM_CHILD:
			var->nchilds = atoi((const char*)r->value->value);
			break;

		case IDV_EXP:
			delete var->exp;
			var->exp = (char*)r->value->value;
			r->value->value = 0;
			break;

		case IDV_ARG:
			var->argument = true;
			break;

		case IDV_INSCOPE:
			var->inscope = ((char*)(r->value->value))[0] == 't' ? true : false;
			break;

		case IDV_CHILDS:
			list_for_each(((gdb_result_t*)r->value->value), c){
				if(c->var_id == IDV_CHILD){
					child = new gdb_variable_t;

					result_to_variable((gdb_result_t*)c->value->value, (void**)&child);

					list_add_tail(&var->childs, child);
					child->parent = var;
				}
			}
			break;
		};
	}

	if(var->type == 0){
		var->type = new char[1];
		var->type[0] = 0;
	}

	if(var->value == 0){
		var->value = new char[1];
		var->value[0] = 0;
	}

	return var->name == 0 ? -1 : 0;
}

int gdb_variable_t::result_to_change_list(gdb_result_t* result, void** unused){
	gdb_variable_t* var;
	gdb_result_t* r;
	gdb_value_t* v;


	if(result->var_id != IDV_CHANGELIST)
		return -1;

	list_for_each((gdb_value_t*)result->value->value, v){
		list_for_each((gdb_result_t*)v->value, r){
			switch(r->var_id){
			case IDV_NAME:
				var = MAP_LOOKUP(gdb_var_lst, (char*)r->value->value);

				if(var == 0){
					ERROR("variable \"%s\" not found in variable list\n", r->value->value);
					continue;
				}

				if(var->parent == 0 || var->parent->childs_visible)
					var->modified = true;

				break;

			case IDV_INSCOPE:
				if(var != 0)
					var->inscope = ((char*)(r->value->value))[0] == 't' ? true : false;
				break;
			};
		}
	}

	return 0;
}

int gdb_variable_t::set(int argc, char** argv){
	if(gdb->mi_issue_cmd((char*)"var-assign",  RC_DONE, 0, 0, "\"%s\" \"%ss %d\"", name, argv, argc) != 0)
		return -1;

	modified = true;
	return 0;
}

int gdb_variable_t::update(){
	gdb_variable_t* v;


	if(modified && inscope){
		v = this;

		if(gdb->mi_issue_cmd((char*)"var-evaluate-expression", RC_DONE, gdb_variable_t::result_to_variable, (void**)&v, "\"%s\"", name) != 0)
			return -1;
	}

	return 0;
}

int gdb_variable_t::print(int win_id, unsigned int* line, map<unsigned int, gdb_variable_t*>* line_map, bool expand, unsigned int indent){
	if(!expand){
		if(update() != 0)
			return -1;

		ui->win_print(win_id, "%s%s = %s", (modified ? "`" : ""), exp, value);
		modified = false;

		return 0;
	}
	else
		return print(win_id, indent, line, line_map);
}

int gdb_variable_t::init_childs(){
	gdb_variable_t *v, *c;


	if(childs != 0)
		return 0;

	v = this;

	if(gdb->mi_issue_cmd((char*)"var-list-children", RC_DONE, gdb_variable_t::result_to_variable, (void**)&v, "\"%s\"", name) != 0)
		return -1;

	list_for_each(childs, c){
		c->origin = origin;
		gdb_var_lst[c->name] = c;
	}

	return 0;
}

int gdb_variable_t::print(int win_id, int rec_lvl, unsigned int* line, map<unsigned int, gdb_variable_t*>* line_map){
	char rec_s[rec_lvl + 1];
	gdb_variable_t* v;


	/* assemble blank string */
	memset(rec_s, ' ', rec_lvl);
	rec_s[rec_lvl] = 0;

	/* update variable value */
	if(update() != 0)
		return -1;

	/* update UI */
	ui->win_print(win_id, "%s%s%s%s = %s\n", rec_s, (nchilds == 0 ? "   " : (childs_visible ? "[-]" : "[+]")), (modified ? "`" : " "), exp, value);

	/* update variable structs */
	modified = false;
	(*line_map)[(*line)++] = this;

	/* print childs */
	if(childs_visible){
		list_for_each(childs, v)
			v->print(win_id, rec_lvl + 1, line, line_map);
	}

	if(rec_lvl == 1 && childs_visible){
		ui->win_print(win_id, "\n");
		(*line)++;
	}

	return 0;
}

#include <common/map.h>
#include <common/list.h>
#include <common/log.h>
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
	modified = false;
	childs_visible = false;
	nchilds = 0;
	origin = O_UNKNOWN;
}

gdb_variable_t::~gdb_variable_t(){
	gdb_variable_t* c;


	gdb_var_lst.erase(gdb_var_lst.find(name));

	delete name;
	delete exp;
	delete type;
	delete value;

	list_for_each(childs, c){
		list_rm(&childs, c);
		delete c;
	}
}

int gdb_variable_t::create(char* expr, gdb_varorigin_t origin){
	gdb_variable_t* v;


	v = this;

	if(gdb->mi_issue_cmd((char*)"var-create", RC_DONE, result_to_variable, (void**)&v, "- %s %s", (origin == O_STACK ? "*" : "@"), expr) == 0){
		if(gdb->mi_issue_cmd((char*)"var-info-expression", RC_DONE, result_to_variable, (void**)&v, "%s", name) == 0)
			return 0;
	}

	return -1;
}

int gdb_variable_t::destroy(){
	if(gdb->mi_issue_cmd((char*)"var-delete", RC_DONE, 0, 0, "%s", name) != 0)
		return -1;
	return 0;
}

int gdb_variable_t::update(){
	gdb_variable_t* v;


	if(modified){
		v = this;

		if(gdb->mi_issue_cmd((char*)"var-evaluate-expression", RC_DONE, result_to_variable, (void**)&v, "%s", name) != 0)
			return -1;
	}

	return 0;
}

int gdb_variable_t::set(int argc, char** argv){
	if(gdb->mi_issue_cmd((char*)"var-assign", RC_DONE, 0, 0, "%s \"%ss %d\"", name, argv, argc) != 0)
		return -1;

	modified = true;
	return 0;
}

int gdb_variable_t::init_childs(){
	gdb_variable_t *v, *c;


	v = this;

	if(childs != 0)
		return 0;

	if(gdb->mi_issue_cmd((char*)"var-list-children", RC_DONE, result_to_variable, (void**)&v, "%s", name) != 0)
		return -1;

	list_for_each(childs, c){
		c->origin = origin;
		if(gdb->mi_issue_cmd((char*)"var-evaluate-expression", RC_DONE, result_to_variable, (void**)&c, "%s", c->name) != 0)
			return -1;
	}

	return 0;
}


/* global functions */
int gdb_variables_update(){
	gdb->mi_issue_cmd((char*)"var-update", RC_DONE, result_to_change_list, 0, "*");
	return 0;
}

int result_to_variable(gdb_result_t* result, void** _var){
	gdb_variable_t *var, *child;
	gdb_result_t *r, *c;


	if(*_var == 0)
		*_var = new gdb_variable_t;

	var = (gdb_variable_t*)*_var;

	list_for_each(result, r){
		switch(r->var_id){
		case IDV_NAME:
			var->name = (char*)r->value->value;
			r->value->value = 0;
			gdb_var_lst[var->name] = var;
			break;

		case IDV_TYPE:
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

		case IDV_CHILDS:
			list_for_each(((gdb_result_t*)r->value->value), c){
				if(c->var_id == IDV_CHILD){
					child = 0;
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

int result_to_change_list(gdb_result_t* result, void** unused){
	gdb_variable_t* var;
	gdb_result_t* r;
	gdb_value_t* v;


	if(result->var_id != IDV_CHANGELIST)
		return -1;

	list_for_each((gdb_value_t*)result->value->value, v){
		list_for_each((gdb_result_t*)v->value, r){
			if(r->var_id == IDV_NAME){
				var = MAP_LOOKUP(gdb_var_lst, (char*)r->value->value);

				if(var == 0){
					ERROR("variable \"%s\" not found in variable list\n", r->value->value);
					continue;
				}

				if(var->parent == 0 || var->parent->childs_visible)
					var->modified = true;
			}
		}
	}

	return 0;
}

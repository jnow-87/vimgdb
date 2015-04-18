#include <common/list.h>
#include <common/log.h>
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


/* global functions */
int gdb_variables_update(gdbif* gdb){
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
	map<string, gdb_variable_t*>::iterator it;


	if(result->var_id != IDV_CHANGELIST)
		return -1;

	list_for_each((gdb_value_t*)result->value->value, v){
		list_for_each((gdb_result_t*)v->value, r){
			if(r->var_id == IDV_NAME){
				it = gdb_var_lst.find((char*)r->value->value);

				if(it == gdb_var_lst.end()){
					ERROR("variable \"%s\" not found in variable list\n", r->value->value);
					continue;
				}

				var = it->second;

				if(var->parent == 0 || var->parent->childs_visible)
					var->modified = true;
			}
		}
	}

	return 0;
}

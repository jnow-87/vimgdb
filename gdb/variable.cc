#include <common/list.h>
#include <common/log.h>
#include <gdb/variable.h>
#include <gdb/variablelist.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <string>


using namespace std;


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

	list_init(this);
}

gdb_variable_t::~gdb_variable_t(){
	gdb_variable_t* c;

	delete name;
	delete exp;
	delete type;
	delete value;

	list_for_each(childs, c){
		list_rm(&childs, c);
		delete c;
	}
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

int result_to_change_list(gdb_result_t* result, void** _var_lst){
	gdb_variablelist* var_lst;
	gdb_variable_t* var;
	gdb_result_t* r;
	gdb_value_t* v;


	var_lst = (gdb_variablelist*)_var_lst;

	if(result->var_id != IDV_CHANGELIST)
		return -1;

	list_for_each((gdb_value_t*)result->value->value, v){
		list_for_each((gdb_result_t*)v->value, r){
			if(r->var_id == IDV_NAME){
				var = var_lst->find((char*)r->value->value);

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

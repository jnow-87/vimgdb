#include <common/log.h>
#include <common/map.h>
#include <common/list.h>
#include <common/string.h>
#include <gui/gui.h>
#include <gdb/gdb.h>
#include <gdb/variable.h>
#include <gdb/strlist.h>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <string>


using namespace std;


/* global variables */
map<string, gdb_variable_t*> gdb_user_var;
map<string, gdb_variable_t*> gdb_callstack_var;
map<string, gdb_variable_t*> gdb_register_var;


/* static variables */
static map<string, gdb_variable_t*> gdb_var_lst;


/* class */
gdb_variable_t::gdb_variable_t(){
	name = 0;
	exp = 0;
	type = 0;
	value = 0;
	parent = 0;
	modified = true;
	childs_visible = false;
	inscope = 't';
	argument = false;
	nchilds = 0;
	origin = O_UNKNOWN;
	refcnt = 1;
}

gdb_variable_t::~gdb_variable_t(){
	if(name && origin != O_UNKNOWN){
		MAP_ERASE(gdb_var_lst, name);

		switch(origin){
		case O_CALLSTACK:
			MAP_ERASE(gdb_callstack_var, name);
			break;

		case O_REGISTER:
			MAP_ERASE(gdb_register_var, name);
			break;

		case O_USER:
			MAP_ERASE(gdb_user_var, name);
			break;

		case O_UNKNOWN:
			break;
		};
	}

	delete [] name;
	delete [] exp;
	delete [] type;
	delete [] value;

	erase_childs();
}

gdb_variable_t* gdb_variable_t::acquire(){
	return new gdb_variable_t;
}

gdb_variable_t* gdb_variable_t::acquire(char* expr, gdb_origin_t origin, char* context, unsigned int frame){
	gdb_variable_t *var, *v;
	string key;


	/* identify variable origin and set key accordingly */
	switch(origin){
	case O_CALLSTACK:
		key = "s:";
		key += context;
		key += ":";
		break;

	case O_REGISTER:
		key = "r:";
		break;

	case O_USER:
		key = "u:";
		break;

	case O_UNKNOWN:
		return 0;
	};

	key += expr;

	/* check if variables already exists */
	var = MAP_LOOKUP(gdb_var_lst, key);

	if(var != 0){
		var->refcnt++;
		return var;
	}

	/* create variable */
	if(gdb->threadid() == 0){
		// create variable without inferior being started (no threadid available)
		if(gdb->mi_issue_cmd("var-create", (gdb_result_t**)&var, "\"%s\" %s %s%s", key.c_str(), (origin == O_USER  ? "@" : "*"), (origin == O_REGISTER ? "$" : ""), expr) != 0)
			return 0;
	}
	else{
		// create variable in the context of the current thread and frame
		if(gdb->mi_issue_cmd("var-create", (gdb_result_t**)&var, "--thread %u --frame %u \"%s\" %s %s%s", gdb->threadid(), frame, key.c_str(), (origin == O_USER  ? "@" : "*"), (origin == O_REGISTER ? "$" : ""), expr) != 0)
			return 0;
	}

	var->origin = origin;
	gdb_var_lst[key] = var;

	switch(origin){
	case O_CALLSTACK:
		gdb_callstack_var[key] = var;
		break;

	case O_REGISTER:
		gdb_register_var[key] = var;
		break;

	case O_USER:
		gdb_user_var[key] = var;
		break;

	case O_UNKNOWN:
		break;
	};

	if(gdb->mi_issue_cmd("var-info-expression", (gdb_result_t**)&v, "\"%s\"", var->name) == 0){
		var->exp = v->exp;
		v->exp = 0;
		delete v;

		return var;
	}

	return 0;
}

int gdb_variable_t::release(gdb_variable_t* v){
	if(--v->refcnt == 0){
		if(v->name && MAP_LOOKUP(gdb_var_lst, v->name) != 0){
			if(gdb->mi_issue_cmd("var-delete", 0, "\"%s\"", v->name) != 0){
				ERROR("var-delete \"%s\" failed, check if this is due to gdb crash "
					  "or an implementation error using non-gdb variable objects\n", v->name);
				return -1;
			}
		}

		delete v;
	}

	return 0;
}

int gdb_variable_t::get_changed(){
	gdb_variable_t *cl, *v, *var;


	if(gdb->mi_issue_cmd("var-update", (gdb_result_t**)&cl, "*") != 0)
		return -1;

	list_for_each(cl, v){
		var = MAP_LOOKUP(gdb_var_lst, v->name);

		if(var == 0){
			ERROR("variable \"%s\" not found in variable list\n", v->name);
			continue;
		}

		if(var->parent == 0 || var->parent->childs_visible)
			var->modified = true;

		if(v->type_changed){
			delete [] var->type;
			var->type = v->type;
			v->type = 0;

			var->nchilds = v->nchilds;

			var->erase_childs();
			var->init_childs();
		}

		if(v->inscope == 'i'){
			if(gdb_variable_t::release(var) != 0)
				goto err;
		}

		list_rm(&cl, v);
		delete v;
	}

	return 0;

err:
	list_for_each(cl, v){
		list_rm(&cl, v);
		delete v;
	}

	return -1;
}

int gdb_variable_t::set(int argc, char** argv){
	if(gdb->mi_issue_cmd("var-assign", 0, "\"%s\" \"%ss %d\"", name, argv, argc) != 0)
		return -1;
		
	modified = true;
	return 0;
}

int gdb_variable_t::format(const char* fmt){
	if(gdb->mi_issue_cmd("var-set-format",  0, "\"%s\" %s", name, fmt) != 0)
		return -1;

	modified = true;
	return 0;
}

int gdb_variable_t::update(){
	gdb_strlist_t* s;


	if(modified && inscope == 't'){
		if(gdb->mi_issue_cmd("var-evaluate-expression", (gdb_result_t**)&s, "\"%s\"", name) != 0)
			return -1;

		delete [] value;
		value = s->s;
		s->s = 0;

		delete s;
	}

	return 0;
}

int gdb_variable_t::print(int win_id, unsigned int* line, map<unsigned int, gdb_variable_t*>* line_map, bool expand, unsigned int indent){
	if(!expand){
		if(update() != 0)
			return -1;

		ui->win_print(win_id, "%s%s = %s%s", (modified ? "´c" : ""), exp, value, (modified ? "`c" : ""));
		modified = false;

		return 0;
	}
	else
		return print(win_id, indent, line, line_map);
}

int gdb_variable_t::init_childs(){
	gdb_variable_t *clst, *c;


	if(childs.size() != 0)
		return 0;

	if(gdb->mi_issue_cmd("var-list-children", (gdb_result_t**)&clst, "\"%s\"", name) != 0)
		return -1;

	list_for_each(clst, c){
		c->origin = origin;
		c->parent = this;
		childs.push_back(c);

		gdb_var_lst[c->name] = c;
	}

	return 0;
}

void gdb_variable_t::erase_childs(){
	list<gdb_variable_t*>::iterator it;


	for(it=childs.begin(); it!=childs.end(); ){
		// childs have no gdb variable object hence
		// they are only deleted, not released
		delete *it;

		it = childs.erase(it);
	}
}

int gdb_variable_t::print(int win_id, int rec_lvl, unsigned int* line, map<unsigned int, gdb_variable_t*>* line_map){
	char rec_s[rec_lvl + 1];
	list<gdb_variable_t*>::iterator it;


	/* assemble blank string */
	memset(rec_s, ' ', rec_lvl);
	rec_s[rec_lvl] = 0;

	/* update variable value */
	if(update() != 0)
		return -1;

	/* update UI */
	ui->win_print(win_id, "%s%s %s%s = %s%s\n", rec_s, (nchilds == 0 ? "   " : (childs_visible ? "[-]" : "[+]")), (modified ? "´c" : ""), exp, value, (modified ? "`c" : ""));

	/* update variable structs */
	modified = false;
	(*line_map)[(*line)++] = this;

	/* print childs */
	if(childs_visible){
		for(it=childs.begin(); it!=childs.end(); it++)
			(*it)->print(win_id, rec_lvl + 1, line, line_map);
	}

	if(rec_lvl == 1 && childs_visible){
		ui->win_print(win_id, "\n");
		(*line)++;
	}

	return 0;
}

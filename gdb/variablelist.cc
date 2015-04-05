#include <gdb/variablelist.h>
#include <gui/gui.h>
#include <stdlib.h>
#include <string.h>


void gdb_variablelist::add(char* name, gdb_variable_t* var){
	smap[name] = var;
}

void gdb_variablelist::add(unsigned int line, gdb_variable_t* var){
	lmap[line] = var;
}

int gdb_variablelist::rm(gdb_variable_t* var){
	gdb_variable_t *child;


	list_for_each(var->childs, child)
		rm(child);

	sit = smap.find(var->name);

	if(sit == smap.end())
		return -1;

	smap.erase(sit);
	return 0;
}

gdb_variable_t* gdb_variablelist::find(char* s){
	sit = smap.find(s);

	return sit != smap.end() ? sit->second : find(atoi(s));
}

gdb_variable_t* gdb_variablelist::find(unsigned int line){
	lit = lmap.find(line);

	return lit == lmap.end() ? 0 : lit->second;
}

void gdb_variablelist::print(gdbif* gdb){
	int win_id_var, i;


	win_id_var = ui->win_getid("variables");

	if(win_id_var < 0)
		return;

	// TODO use atomicStart for the loop
	ui->win_clear(win_id_var);
	lmap.clear();

	i = 1;
	for(sit=smap.begin(); sit!=smap.end(); sit++){
		if(sit->second->parent == 0)
			print(gdb, sit->second, &i, win_id_var, 1);
	}
}

void gdb_variablelist::print(gdbif* gdb, gdb_variable_t* var, int* line, int win_id, int rec_lvl){
	char rec_s[rec_lvl + 1];
	gdb_variable_t* v;


	/* assemble blank string */
	memset(rec_s, ' ', rec_lvl);
	rec_s[rec_lvl] = 0;

	/* update variable value */
	if(var->modified)
		gdb->mi_issue_cmd((char*)"var-evaluate-expression", RC_DONE, result_to_variable, (void**)&var, "%s", var->name);

	/* update UI */
	ui->win_print(win_id, "%s%s%s %s = %s\n", (var->modified ? "*" : " "), rec_s, (var->nchilds == 0 ? "   " : (var->childs_visible ? "[-]" : "[+]")), var->exp, var->value);

	/* update variable structs */
	var->modified = false;
	lmap[*line] = var;
	(*line)++;

	/* print childs */
	if(var->childs_visible){
		list_for_each(var->childs, v)
			print(gdb, v, line, win_id, rec_lvl + 1);
	}

	if(rec_lvl == 1){
		ui->win_print(win_id, "\n");
		(*line)++;
	}
}

void gdb_variablelist::update(gdbif* gdb){
	if(ui->win_getid("variables") < 0)
		return;

	gdb->mi_issue_cmd((char*)"var-update", RC_DONE, result_to_change_list, (void**)this, "*");
}

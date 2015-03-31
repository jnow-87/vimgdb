#ifndef GDB_VARIABLELIST_H
#define GDB_VARIABLELIST_H


#include <gdb/gdb.h>
#include <gdb/variable.h>
#include <map>
#include <string>


using namespace std;


/* classes */
class gdb_variablelist{
public:
	void add(char* name, gdb_variable_t* var);
	void add(unsigned int line, gdb_variable_t* var);

	int rm(gdb_variable_t* var);

	gdb_variable_t* find(char* s);
	gdb_variable_t* find(unsigned int line);

	void print(gdbif* gdb);
	void update(gdbif* gdb);

private:
	void print(gdbif* gdb, gdb_variable_t* var, int* line, int win_id, int rec_lvl);

	map<string, gdb_variable_t*> smap;
	map<unsigned int, gdb_variable_t*> lmap;

	map<string, gdb_variable_t*>::iterator sit;
	map<unsigned int, gdb_variable_t*>::iterator lit;
};


#endif // GDB_VARIABLELIST_H

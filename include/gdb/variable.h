#ifndef GDB_VARIABLE_H
#define GDB_VARIABLE_H


#include <gdb/result.h>
#include <gdb/gdb.h>
#include <map>
#include <string>


using namespace std;


/* types */
typedef enum{
	O_UNKNOWN = 0,
	O_STACK,
	O_USER,
} gdb_varorigin_t;


/* class */
class gdb_variable_t{
public:
	static gdb_variable_t* acquire(char* expr, char* context = 0, unsigned int frame = 0);
	static int release(gdb_variable_t* v);
	static int get_changed();

	static int result_to_variable(gdb_result_t* result, void** var);
	static int result_to_change_list(gdb_result_t* result, void** unused);


	int set(int argc, char** argv);
	int update();
	int print(int win_id, unsigned int* line, map<unsigned int, gdb_variable_t*>* line_map, bool expand, unsigned int indent = 0);
	int init_childs();

	char *name,
		 *exp,
		 *type,
		 *value;

	bool modified,
		 childs_visible,
		 inscope,
		 argument;

	unsigned int nchilds,
				 refcnt;

	gdb_varorigin_t origin;

	class gdb_variable_t *next,
						 *prev,
						 *parent,
						 *childs;

private:
	/* to force usage of acquire() and release() */
	gdb_variable_t();
	~gdb_variable_t();

	int print(int win_id, int rec_lvl, unsigned int* line, map<unsigned int, gdb_variable_t*>* line_map);
};


/* external variables */
extern map<string, gdb_variable_t*> gdb_var_lst;


#endif // GDB_VARIABLE_H

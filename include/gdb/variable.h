#ifndef GDB_VARIABLE_H
#define GDB_VARIABLE_H


#include <common/dynarray.h>
#include <gdb/result.h>
#include <map>
#include <list>
#include <string>


using namespace std;


/* types */
typedef enum{
	O_UNKNOWN = 0,
	O_CALLSTACK,
	O_REGISTER,
	O_USER,
} gdb_origin_t;


/* class */
class gdb_variable_t : public gdb_result_t{
public:
	static gdb_variable_t *acquire();
	static gdb_variable_t *acquire(char *expr, gdb_origin_t origin, char *context = 0, unsigned int frame = 0);
	static int release(gdb_variable_t *v);
	static int get_changed();


	int set(int argc, char **argv);
	int format(const char *fmt);
	int update();
	int print(dynarray *obuf, unsigned int *line, map<unsigned int, gdb_variable_t*>* line_map, bool expand, unsigned int indent = 0);
	int init_childs();

	char *name,
		 *exp,
		 *type,
		 *value;

	char inscope;
	bool modified,
		 childs_visible,
		 argument,
		 type_changed;

	unsigned int nchilds,
				 refcnt;

	gdb_origin_t origin;

	list<class gdb_variable_t*> childs;

	class gdb_variable_t *next,
						 *prev,
						 *parent;

private:
	/* to force usage of acquire() and release() */
	gdb_variable_t();
	~gdb_variable_t();

	int print(dynarray *obuf, int rec_lvl, unsigned int *line, map<unsigned int, gdb_variable_t*>* line_map);
	void erase_childs();
};


/* external variables */
extern map<string, gdb_variable_t*> gdb_user_var;
extern map<string, gdb_variable_t*> gdb_callstack_var;
extern map<string, gdb_variable_t*> gdb_register_var;


#endif // GDB_VARIABLE_H

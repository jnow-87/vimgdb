#ifndef GDB_FRAME_H
#define GDB_FRAME_H


#include <gdb/result.h>
#include <gdb/variable.h>
#include <list>


using namespace std;


class gdb_frame_t : public gdb_result_t{
public:
	static gdb_frame_t *acquire();
	static gdb_frame_t *acquire(char *name, char *context, gdb_frame_t *src = 0);
	static void release(gdb_frame_t *f);


	void *addr;
	unsigned int line,
				 level;

	char *function,
		 *filename,
		 *fullname,
		 *from;

	bool expanded;
	char *context;

	list<gdb_variable_t*> args,
						  locals;

	gdb_frame_t *next,
				*prev;

private:
	gdb_frame_t();
	~gdb_frame_t();
};


#endif // GDB_FRAME_H

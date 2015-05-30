#ifndef GDB_FRAME_H
#define GDB_FRAME_H


#include <gdb/result.h>
#include <gdb/variable.h>
#include <list>


using namespace std;


class gdb_frame_t : public gdb_result_t{
public:
	gdb_frame_t();
	~gdb_frame_t();

	void* addr;
	unsigned int line,
				 level;
	
	char *function,
		 *filename,
		 *fullname;

	bool expanded;

	list<gdb_variable_t*> args,
						  locals;

	gdb_frame_t *next,
				*prev;
};


#endif // GDB_FRAME_H

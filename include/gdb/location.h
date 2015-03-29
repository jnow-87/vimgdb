#ifndef LOCATION_H
#define LOCATION_H


#include <gdb/result.h>


/* types */
class gdb_location_t{
public:
	gdb_location_t();
	~gdb_location_t();

	char *fullname,
		 *filename;

	unsigned int line;
};


/* prototypes */
int result_to_location(gdb_result_t* result, void** loc);


#endif // LOCATION_H

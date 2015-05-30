#ifndef LOCATION_H
#define LOCATION_H


#include <gdb/result.h>


class gdb_location_t : public gdb_result_t{
public:
	gdb_location_t();
	~gdb_location_t();

	char *fullname,
		 *filename;

	unsigned int line;
};


#endif // LOCATION_H

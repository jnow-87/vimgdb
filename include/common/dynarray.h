#ifndef DYNARRAY_H
#define DYNARRAY_H


#include <stdarg.h>


class dynarray{
public:
	dynarray();
	~dynarray();

	int add(const char* fmt, ...);
	void clear();

	const char* data();

private:
	unsigned int len,
				 idx;

	char* s;
};


#endif // DYNARRAY_H

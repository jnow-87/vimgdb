#ifndef PERTYPES_H
#define PERTYPES_H


#include <gdb/memory.h>


class per_bits_t{
public:
	per_bits_t(char* name, unsigned int idx, unsigned int nbits);
	~per_bits_t();

	char* name;
	unsigned int idx,
				 nbits,
				 mask;

	unsigned long int value;

	class per_bits_t *next,
					 *prev;
};

class per_register_t{
public:
	per_register_t(char* name, unsigned int offset, unsigned int nbytes, per_bits_t* bits);
	~per_register_t();

	char* name;
	unsigned int offset,
				 nbytes;

	per_bits_t* bits;

	class per_range_t* parent;
	class per_register_t *next,
						 *prev;
};

class per_range_t{
public:
	per_range_t(char* name, unsigned long int base, unsigned int size, per_register_t* regs);
	~per_range_t();

	char* name;
	unsigned long int base;
	unsigned int size;
	bool expanded;

	per_register_t* regs;

	gdb_memory_t* mem;

	class per_range_t *next,
					  *prev;
};


#endif // PERTYPES_H

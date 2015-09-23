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
	per_register_t(char* name, char* desc, unsigned int offset, unsigned int nbytes, per_bits_t* bits);
	~per_register_t();

	char *name,
		 *desc;

	unsigned int offset,
				 nbytes;

	per_bits_t* bits;

	class per_range_t* parent;
	class per_register_t *next,
						 *prev;
};

class per_range_t{
public:
	per_range_t(char* name, void* base, unsigned int size, per_register_t* regs);
	~per_range_t();

	char* name;
	void* base;
	unsigned int size;

	per_register_t* regs;

	gdb_memory_t* mem;

	class per_range_t *next,
					  *prev;
};

class per_section_t{
public:
	per_section_t(char* name, per_range_t* ranges);
	~per_section_t();

	char* name;
	bool expanded;
	per_range_t* ranges;

	class per_section_t *next,
						*prev;
};


#endif // PERTYPES_H

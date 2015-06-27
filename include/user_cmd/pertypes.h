#ifndef PER_H
#define PER_H


class per_bits_t{
public:
	per_bits_t(char* name, unsigned int idx, unsigned int nbits);
	~per_bits_t();

	char* name;
	unsigned int idx,
				 nbits;

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

	class per_range_t *next,
					  *prev;
};


#endif // PER_H

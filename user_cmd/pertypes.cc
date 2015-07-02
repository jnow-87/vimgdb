#include <common/list.h>
#include <user_cmd/pertypes.h>


per_bits_t::per_bits_t(char* name, unsigned int idx, unsigned int nbits){
	unsigned int i;


	this->name = name;
	this->idx = idx;
	this->nbits = nbits;
	this->value = 0;

	this->mask = 0;
	for(i=0; i<nbits; i++)
		this->mask |= 0x1 << i;
}

per_bits_t::~per_bits_t(){
	delete [] name;
}

per_register_t::per_register_t(char* name, unsigned int offset, unsigned int nbytes, per_bits_t* bits){
	this->name = name;
	this->offset = offset;
	this->nbytes = nbytes;
	this->bits = bits;
	this->parent = 0;
}

per_register_t::~per_register_t(){
	per_bits_t* b;


	delete [] name;

	list_for_each(bits, b){
		list_rm(&bits, b);
		delete b;
	}
}

per_range_t::per_range_t(char* name, void* base, unsigned int size, per_register_t* regs){
	this->name = name;
	this->base = base;
	this->size = size;
	this->regs = regs;
	this->mem = 0;
	this->expanded = false;
}

per_range_t::~per_range_t(){
	per_register_t* reg;


	delete [] name;
	delete mem;

	list_for_each(regs, reg){
		list_rm(&regs, reg);
		delete reg;
	}
}

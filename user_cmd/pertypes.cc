#include <user_cmd/pertypes.h>


per_bits_t::per_bits_t(char* name, unsigned int idx, unsigned int nbits){
	this->name = name;
	this->idx = idx;
	this->nbits = nbits;
}

per_bits_t::~per_bits_t(){
	delete [] name;
}

per_register_t::per_register_t(char* name, unsigned int offset, unsigned int nbytes, per_bits_t* bits){
	this->name = name;
	this->offset = offset;
	this->nbytes = nbytes;
	this->bits = bits;
}

per_register_t::~per_register_t(){
	delete [] name;
}

per_range_t::per_range_t(char* name, void* base, unsigned int size, per_register_t* regs){
	this->name = name;
	this->base = base;
	this->size = size;
	this->regs = regs;
}

per_range_t::~per_range_t(){
	delete [] name;
}

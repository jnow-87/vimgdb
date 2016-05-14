#include <common/list.h>
#include <user_cmd/pertypes.h>


per_bits_t::per_bits_t(char *name, unsigned int idx, unsigned int nbits){
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

per_register_t::per_register_t(char *name, char *desc, unsigned int offset, unsigned int nbytes, per_reg_opt_t opt, per_bits_t *bits){
	this->name = name;
	this->desc = desc;
	this->offset = offset;
	this->nbytes = nbytes;
	this->opt = opt;
	this->bits = bits;
	this->parent = 0;
}

per_register_t::~per_register_t(){
	per_bits_t *b;


	delete [] name;
	delete [] desc;

	list_for_each(bits, b){
		list_rm(&bits, b);
		delete b;
	}
}

per_section_t::per_section_t(char *name, per_register_t *regs){
	this->name = name;
	this->regs = regs;
	this->expanded = false;
}

per_section_t::~per_section_t(){
	per_register_t *reg;


	delete [] name;

	list_for_each(regs, reg){
		list_rm(&regs, reg);
		delete reg;
	}
}

per_range_t::per_range_t(void *base, unsigned int size, per_section_t *sections){
	this->base = base;
	this->size = size;
	this->sections = sections;
	this->mem = 0;
}

per_range_t::~per_range_t(){
	per_section_t *sec;


	delete mem;

	list_for_each(sections, sec){
		list_rm(&sections, sec);
		delete sec;
	}
}

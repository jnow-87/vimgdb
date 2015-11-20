#include <common/log.h>
#include <common/list.h>
#include <gdb/gdb.h>
#include <gdb/memory.h>
#include <stdlib.h>
#include <string.h>


/* macros */
// convert character to string containing its hex representation
#define CTOHEXSTR(c, s){ \
	int v; \
	\
	\
	v = (int)(c) / 16; \
	(s)[0] = (char)(v + (v > 9 ? 87 : 48)); \
	v = (int)(c) % 16; \
	(s)[1] = (char)(v + (v > 9 ? 87 : 48)); \
}


/* global functions */
gdb_memory_t::gdb_memory_t(){
	begin = 0;
	content = 0;
	content_old = 0;
	length = 0;
	expanded = true;
	alignment = 8;
	next = 0;
	prev = 0;
}

gdb_memory_t::~gdb_memory_t(){
	delete [] begin;
	delete [] content;
	delete [] content_old;
}

gdb_memory_t* gdb_memory_t::acquire(){
	return new gdb_memory_t;
}

gdb_memory_t* gdb_memory_t::acquire(void* addr, unsigned int length){
	gdb_memory_t* m;


	if(gdb->mi_issue_cmd("data-read-memory-bytes", (gdb_result_t**)&m, "0x%lx %u", addr, length) != 0)
		return 0;
	return m;
}

gdb_memory_t* gdb_memory_t::acquire(char* addr, unsigned int length){
	gdb_memory_t* m;


	if(gdb->mi_issue_cmd("data-read-memory-bytes", (gdb_result_t**)&m, "%s %u", addr, length) != 0)
		return 0;
	return m;
}

int gdb_memory_t::set(void* addr, char* value, unsigned int cnt){
	char* p;
	int r;
	unsigned int len;


	/* check format of value */
	p = 0;

	if(value[0] == '"'){
		// convert quoted string to hex representation
		len = strlen(value);

		if(value[len - 1] != '"'){
			USER("expected closing \"\n");
			return -1;
		}

		len -= 2;	// skip enclosing '"'
		p = new char[len * 2 + 1];
		p[len * 2] = 0;

		for(; len>0; len--)
			CTOHEXSTR(value[len], p + len * 2 - 2);
	}
	else if(value[0] == '\''){
		// convert char to hex representation
		CTOHEXSTR(value[1], value);
		value[2] = 0;
	}
	else if(value[0] == '0' && value[1] == 'x')
		// skip "0x"
		value += 2;

	/* update memory */
	if(cnt) r = gdb->mi_issue_cmd("data-write-memory-bytes", 0, "0x%lx %s %u", addr, (p ? p : value), cnt);
	else	r = gdb->mi_issue_cmd("data-write-memory-bytes", 0, "0x%lx %s", addr, (p ? p : value));

	if(p)
		delete [] p;

	if(r)
		return -1;
	return 0;
}

int gdb_memory_t::update(){
	gdb_memory_t* m;


	if(gdb->mi_issue_cmd("data-read-memory-bytes", (gdb_result_t**)&m, "%s %u", begin, length) != 0)
		return -1;

	delete [] content_old;
	content_old = content;
	content = m->content;

	m->content = 0;
	delete m;

	return 0;
}

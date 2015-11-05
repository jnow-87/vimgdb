#include <common/dynarray.h>
#include <stdlib.h>
#include <stdio.h>


dynarray::dynarray(){
	idx = 0;
	len = 0;

	s = (char*)malloc(1024);

	if(s != 0)
		len = 1024;
}

dynarray::~dynarray(){
	free(s);
}

int dynarray::add(const char* fmt, ...){
	unsigned int i;
	va_list lst;


	while(1){
		va_start(lst, fmt);

		i = vsnprintf(s + idx, len - idx, fmt, lst);

		va_end(lst);

		if(i < len - idx){
			idx += i;
			
			return i;
		}

		len *= 2;
		s = (char*)realloc(s, len);

		if(s == 0)
			return -1;
	}
}

void dynarray::clear(){
	idx = 0;
	s[0] = 0;
}

const char* dynarray::data(){
	return s;
}

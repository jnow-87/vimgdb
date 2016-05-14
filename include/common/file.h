#ifndef COMMON_FILE_H
#define COMMON_FILE_H


#include <stdio.h>


#define FILE_EXISTS(file)({ \
	FILE *fp = fopen(file, "r"); \
	if(fp != 0) \
		fclose(fp); \
	(fp == 0 ? false : true); \
})

#endif

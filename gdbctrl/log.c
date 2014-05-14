#include <stdarg.h>
#include <stdio.h>
#include "log.h"


/* static variables */
static FILE* log_file = 0;
static log_level_t log_level = INFO | ERROR | WARN;


/* global functions */
int log_init(const char* file_name, log_level_t lvl){
	log_level = lvl;
	log_file = fopen(file_name, "w");
	
	if(log_file == 0)
		return -1;
	return 0;
}

void log_cleanup(){
	INFO("closing log file\n");
	if(log_file != 0)
		fclose(log_file);
}

void log_print(log_level_t lvl, const char* msg, ...){
	va_list lst;


	if(lvl & log_level){
		va_start(lst, msg);
		vfprintf(log_file, msg, lst);
		va_end(lst);
	}
}

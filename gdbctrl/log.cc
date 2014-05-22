#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include "log.h"


/* static variables */
FILE* log::log_file = 0;
log_level_t log::log_level = (log_level_t)(INFO | ERROR | WARN);


/* global functions */
int log::init(const char* file_name, log_level_t lvl){
	log_level = lvl;

	if(log_level != NONE && log_file == 0){
		log_file = fopen(file_name, "w");
		
		if(log_file == 0)
			return -1;
	}

	return 0;
}

void log::cleanup(){
	INFO("closing log file\n");
	if(log_file != 0)
		fclose(log_file);
}

void log::print(log_level_t lvl, const char* msg, ...){
	va_list lst;


	if(lvl & log_level && log_file != 0){
		va_start(lst, msg);
		vfprintf(log_file, msg, lst);
		va_end(lst);
	}
}

char* log::stime(){
    static char s[80];
    time_t t;
	tm* ts;


	t = time(0);
	ts = localtime(&t);
	strftime(s, 80, "%d.%m.%Y %H:%M:%S", ts);

    return s;
}

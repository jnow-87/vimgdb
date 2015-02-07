#ifndef LOG_H
#define LOG_H


#include <stdio.h>


/* types */
enum log_level_t{
	NONE = 0x0,
	INFO = 0x1,
	WARN = 0x2,
	ERROR = 0x4,
	DEBUG = 0x8,
	USER = 0x10,
	TEST = 0x20,
	TODO = 0x40,
};


/* macros */
#define INFO(msg, ...)	log::print(INFO, "[INF][%19.19s] %10.10s:%-5d %20.20s(): " msg, log::stime(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define ERROR(msg, ...)	log::print(ERROR, "[ERR][%19.19s] %10.10s:%-5d %20.20s(): " msg, log::stime(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define WARN(msg, ...)	log::print(WARN, "[WAR][%19.19s] %10.10s:%-5d %20.20s(): " msg, log::stime(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define TODO(msg, ...)	log::print(TODO, "[TODO][%19.19s] %10.10s:%-5d %20.20s(): " msg, log::stime(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define DEBUG(msg, ...)	log::print(DEBUG, msg, ##__VA_ARGS__)
#define USER(msg, ...)	log::print(USER, msg, ##__VA_ARGS__)
#define TEST(msg, ...)	log::print(TEST, msg, ##__VA_ARGS__)


/* class */
class log{
public:
	/* init and cleanup function */
	static int init(const char* file_name, log_level_t lvl);
	static void cleanup();

	/* add entry to log */
	static void print(log_level_t lvl, const char* msg, ...);

	/* get current time/date */
	static char* stime();

private:
	static FILE* log_file;			// file pointer to log file
	static log_level_t log_level;	// current log level
};


#endif

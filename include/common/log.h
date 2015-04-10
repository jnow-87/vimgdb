#ifndef LOG_H
#define LOG_H


#include <sys/types.h>
#include <stdio.h>
#include <map>
#include <string>


using namespace std;


/* types */
enum log_level_t{
	NONE = 0x0,
	ERROR = 0x1,
	DEBUG = 0x2,
	USER = 0x4,
	TEST = 0x8,
	TODO = 0x10,
	GDB = 0x20,
	VIM = 0x40,
};


/* macros */
#ifndef LOG_ERROR
#define LOG_ERROR 0
#define ERROR(msg, ...)	log::print(ERROR, "", ##__VA_ARGS__)
#else
#define ERROR(msg, ...)	log::print(ERROR,	" [EE][%19.19s] %15.15s @ %15.15s:%-5d %15.15s(): " msg, log::stime(), thread_name[pthread_self()].c_str(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#endif // LOG_ERROR

#ifndef LOG_TODO
#define LOG_TODO 0
#define TODO(msg, ...)	log::print(TODO, "", ##__VA_ARGS__)
#else
#define TODO(msg, ...)	log::print(TODO,	"[TBD][%19.19s] %15.15s @ %15.15s:%-5d %15.15s(): " msg, log::stime(), thread_name[pthread_self()].c_str(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#endif // LOG_TODO

#ifndef LOG_DEBUG
#define LOG_DEBUG 0
#define DEBUG(msg, ...)	log::print(DEBUG, "", ##__VA_ARGS__)
#else
#define DEBUG(msg, ...)	log::print(DEBUG,	"[DBG][%19.19s] %15.15s @ %15.15s:%-5d %15.15s(): " msg, log::stime(), thread_name[pthread_self()].c_str(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#endif // LOG_DEBUG

#ifndef LOG_GDB
#define LOG_GDB 0
#define GDB(msg, ...)	log::print(GDB, "", ##__VA_ARGS__)
#else
#define GDB(msg, ...)	log::print(GDB,		"[GDB][%19.19s] %15.15s @ %15.15s:%-5d %15.15s(): " msg, log::stime(), thread_name[pthread_self()].c_str(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#endif // LOG_GDB

#ifndef LOG_VIM
#define LOG_VIM 0
#define VIM(msg, ...)	log::print(VIM, "", ##__VA_ARGS__)
#else
#define VIM(msg, ...)	log::print(VIM,		"[VIM][%19.19s] %15.15s @ %15.15s:%-5d %15.15s(): " msg, log::stime(), thread_name[pthread_self()].c_str(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#endif // LOG_VIM

#ifndef LOG_USER
#define LOG_USER 0
#define USER(msg, ...)	log::print(USER, "", ##__VA_ARGS__)
#else
#define USER(msg, ...)	log::print(USER, msg, ##__VA_ARGS__)
#endif // LOG_USER

#ifndef LOG_TEST
#define LOG_TEST 0
#define TEST(msg, ...)	log::print(TEST, "", ##__VA_ARGS__)
#else
#define TEST(msg, ...)	log::print(TEST, msg, ##__VA_ARGS__)
#endif // LOG_TEST


/* external variables */
extern map<pthread_t, string> thread_name;


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
	static int win_id_debug,		// gui window ids
			   win_id_user;
	static log_level_t log_level;	// current log level
	static pid_t creator;			// pid of creating process
};


#endif

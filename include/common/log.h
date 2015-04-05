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
	INFO = 0x1,
	WARN = 0x2,
	ERROR = 0x4,
	DEBUG = 0x8,
	USER = 0x10,
	TEST = 0x20,
	TODO = 0x40,
};


/* macros */
#define INFO(msg, ...)	log::print(INFO,	"[II][%19.19s] %15.15s - %15.15s:%-5d %15.15s(): " msg, log::stime(), thread_name[pthread_self()].c_str(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define ERROR(msg, ...)	log::print(ERROR,	"[EE][%19.19s] %15.15s - %15.15s:%-5d %15.15s(): " msg, log::stime(), thread_name[pthread_self()].c_str(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define WARN(msg, ...)	log::print(WARN,	"[WW][%19.19s] %15.15s - %15.15s:%-5d %15.15s(): " msg, log::stime(), thread_name[pthread_self()].c_str(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define TODO(msg, ...)	log::print(TODO,	"[TD][%19.19s] %15.15s - %15.15s:%-5d %15.15s(): " msg, log::stime(), thread_name[pthread_self()].c_str(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define DEBUG(msg, ...)	log::print(DEBUG,	"[DB][%19.19s] %15.15s - %15.15s:%-5d %15.15s(): " msg, log::stime(), thread_name[pthread_self()].c_str(), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define USER(msg, ...)	log::print(USER, msg, ##__VA_ARGS__)
#define TEST(msg, ...)	log::print(TEST, msg, ##__VA_ARGS__)


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

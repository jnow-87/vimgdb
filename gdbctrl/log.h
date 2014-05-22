#ifndef LOG_H
#define LOG_H


#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>


/* types */
enum log_level_t{
	NONE = 0x0,
	INFO = 0x1,
	WARN = 0x2,
	ERROR = 0x4
};


/* macros */
#define INFO(msg, ...)	log::print(INFO, "[INF][%d][%19.19s] %20.20s(): " msg, getpid(), log::stime(), __FUNCTION__, ##__VA_ARGS__)
#define ERROR(msg, ...)	log::print(ERROR, "[ERR][%d][%19.19s] %20.20s(): " msg, getpid(), log::stime(), __FUNCTION__, ##__VA_ARGS__)
#define WARN(msg, ...)	log::print(WARN, "[WAR][%d][%19.19s] %20.20s(): " msg, getpid(), log::stime(), __FUNCTION__, ##__VA_ARGS__)


/* class */
class log{
public:
	/**
	 * \brief	init log system
	 *
	 * \param	file_name	file name of log file
	 * \param	lvl			log level to apply
	 *
	 * \return	0			success
	 * 			-1			error (check errno)
	 */
	static int init(const char* file_name, log_level_t lvl);

	/**
	 * \brief	safely shut down log system
	 */
	static void cleanup();

	/**
	 * \brief	write log message to log file
	 *
	 * \param	lvl			log level of message
	 * \param	msg			actual message (printf-like format string)
	 * \param	...			arguments as defined in <msg>
	 */
	static void print(log_level_t lvl, const char* msg, ...);

	static char* stime();

private:
	static FILE* log_file;
	static log_level_t log_level;
};


#endif

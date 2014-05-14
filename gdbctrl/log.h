#ifndef LOG_H
#define LOG_H


/* macros */
#define INFO(msg, ...)	log_print(INFO, "[INF][%d] %20s()\t"msg, getpid(), __FUNCTION__, ##__VA_ARGS__)
#define ERROR(msg, ...)	log_print(ERROR, "[ERR][%d] %20s()\t"msg, getpid(), __FUNCTION__, ##__VA_ARGS__)
#define WARN(msg, ...)	log_print(WARN, "[WAR][%d] %20s()\t"msg, getpid(), __FUNCTION__, ##__VA_ARGS__)


/* types */
typedef enum{
	INFO = 0x1,
	WARN = 0x2,
	ERROR = 0x4
} log_level_t;


/* protorypes */
/**
 * \brief	init log system
 *
 * \param	file_name	file name of log file
 * \param	lvl			log level to apply
 *
 * \return	0			success
 * 			-1			error (check errno)
 */
int log_init(const char* file_name, log_level_t lvl);

/**
 * \brief	safely shut down log system
 */
void log_cleanup();

/**
 * \brief	write log message to log file
 *
 * \param	lvl			log level of message
 * \param	msg			actual message (printf-like format string)
 * \param	...			arguments as defined in <msg>
 */
void log_print(log_level_t lvl, const char* msg, ...);


#endif

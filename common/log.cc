#include <config/config.h>
#include <common/defaults.h>
#include <common/log.h>
#include <gui/gui.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>


/* global variables */
map<pthread_t, string> thread_name;


/* static variables */
FILE *log::log_file = 0;
log_level_t log::log_level = (log_level_t)(ERROR | USER);
pid_t log::creator = 0;


/* class definition */
/**
 * \brief	init log system
 *
 * \param	file_name	file name of log file
 * \param	lvl			log level to apply
 *
 * \return	0			success
 * 			-1			error (check errno)
 */
int log::init(const char *file_name, log_level_t lvl){
	log_level = lvl;

	creator = getpid();

	if(log_level != NONE && log_file == 0){
		log_file = fopen(file_name, "a");

		if(log_file == 0)
			return -1;
	}

	if(ui){
#ifdef CONFIG_GUI_CURSES
		if(ui->win_create("user-log", true, 0) < 0)
			goto err_1;

		if(ui->win_create("debug-log", true, 0) < 0)
			goto err_2;
#endif // CONFIG_GUI_CURSES
	}

	return 0;

#ifdef CONFIG_GUI_CURSES

err_2:
	ui->win_destroy(ui->win_getid(USERLOG_NAME));

err_1:
	fclose(log_file);

err_0:
	return -1;

#endif // CONFIG_GUI_CURSES
}

/**
 * \brief	safely shut down log system
 */
void log::cleanup(){
	if(log_file != 0)
		fclose(log_file);

	log_file = 0;

	// ensure that only creating process is closing the windows
	// otherwise any forked process would destroy them
	if(ui && creator == getpid()){

#ifndef CONFIG_VIM_KEEP_USERLOG
		ui->win_destroy(ui->win_getid(USERLOG_NAME));
#endif

#ifdef CONFIG_GUI_CURSES
		ui->win_destroy(ui->win_getid(DEBUGLOG_NAME));
#endif
	}
}

/**
 * \brief	write log message to log file
 *
 * \param	lvl			log level of message
 * \param	msg			actual message (printf-like format string)
 * \param	...			arguments as defined in <msg>
 */
void log::print(log_level_t lvl, const char *msg, ...){
	va_list lst;


	if(lvl & log_level){
		if(log_file){
			va_start(lst, msg);
			vfprintf(log_file, msg, lst);
			fflush(log_file);
			va_end(lst);
		}

		if(ui){
			va_start(lst, msg);

#ifdef CONFIG_GUI_CURSES
			if(lvl & (USER | TEST))	ui->win_vprint(ui->win_getid(USERLOG_NAME), msg, lst);
			else					ui->win_vprint(ui->win_getid(DEBUGLOG_NAME), msg, lst);
#else
			if(lvl & (USER | TEST))	ui->win_vprint(ui->win_getid(USERLOG_NAME), msg, lst);
			else					vprintf(msg, lst);
#endif

			va_end(lst);
		}
	}
}

/**
 * \brief	get current time/date as string
 *
 * \return	pointer to time/date string - string is allocated statically and
 * 			hence must not be freed
 */
char *log::stime(){
	static char s[80];
	time_t t;
	tm *ts;


	t = time(0);
	ts = localtime(&t);
	strftime(s, 80, "%d.%m.%Y %H:%M:%S", ts);

	return s;
}

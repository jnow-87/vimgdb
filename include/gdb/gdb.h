#ifndef GDB_H
#define GDB_H


#include <common/pty.h>
#include <gdb/result.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <map>


using namespace std;


/* marcos */
#define GDB_CMD		"/usr/bin/gdb"
#define GDB_ARGS						/* specified as comma-separated list of strings */ \
					"--interpreter=mi", /* enable MI interface */ \
					"-q" 				/* disbale gdb info on start*/


/* types */
typedef struct{
	gdb_result_class_t rclass;
	gdb_result_t* result;
} gdb_response_t;


/* class */
class gdbif{
public:
	/* constructor/desctructor */
	gdbif();
	~gdbif();

	/* init gdb interface */
	int init();

	/* gdb machine interface (MI) */
	gdb_response_t* mi_issue_cmd(char* cmd, const char* param_fmt, ...);
	int mi_proc_result(gdb_result_class_t rclass, unsigned int token, gdb_result_t* result);
	int mi_proc_async(gdb_result_class_t rclass, unsigned int token, gdb_result_t* result);
	int mi_proc_stream(gdb_stream_class_t sclass, char* stream);

	/* communication with gdb */
	int sigsend(int sig);
	int read(void* buf, unsigned int nbytes);
	int write(void* buf, unsigned int nbytes);

private:
	/* variables */
	// gdb child data
	pty* gdb;
	pid_t pid;

	// token used for gdb commands
	unsigned int token;

	// gdb MI command response handling
	volatile unsigned int resp_token;
	volatile gdb_response_t resp;
	pthread_cond_t resp_avail;
	pthread_mutex_t resp_mtx;
};


#endif

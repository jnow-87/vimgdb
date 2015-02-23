#ifndef GDB_H
#define GDB_H


#include <common/pty.h>
#include <gdb/result.h>
#include <gdb/arglist.h>
#include <pthread.h>
#include <signal.h>
#include <map>


using namespace std;


/* marcos */
#define GDB_CMD		"/usr/bin/gdb"
#define GDB_ARGS						/* specified as comma-separated list of strings */ \
					"--interpreter=mi", /* enable MI interface */ \
					"-q" 				/* disbale gdb info on start*/


/* types */
typedef struct{
	result_class_t rclass;
	result_t* result;
} response_t;


/* class */
class gdbif{
public:
	/* constructor/desctructor */
	gdbif();
	~gdbif();

	/* init gdb interface */
	int init();

	/* gdb machine interface (MI) */
	response_t* mi_issue_cmd(char* cmd, arglist_t* options, arglist_t* parameter);
	int mi_parse(char* s);
	int mi_proc_result(result_class_t rclass, unsigned int token, result_t* result);
	int mi_proc_async(result_class_t rclass, unsigned int token, result_t* result);
	int mi_proc_stream(stream_class_t sclass, char* stream);

	/* communication with gdb */
	int sigsend(int sig);
	int read(void* buf, unsigned int nbytes);
	int write(void* buf, unsigned int nbytes);

private:
	/* variables */
	// gdb child data
	pty* child_term;
	pid_t child_pid;

	// token used for gdb commands
	unsigned int token;

	// gdb MI command response handling
	volatile unsigned int resp_token;
	volatile response_t resp;
	pthread_cond_t resp_avail;
	pthread_mutex_t resp_mtx;
};


#endif

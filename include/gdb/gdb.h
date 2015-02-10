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
typedef int (*response_hdlr_t)(result_class_t rclass, result_t* result, char* cmdline, void* data);

typedef struct{
	char* cmdline;
	void* data;
	response_hdlr_t resp_hdlr;
} mi_cmd_t;


/* class */
class gdbif{
public:
	/* constructor/desctructor */
	gdbif();
	~gdbif();

	/* init gdb interface */
	int init();

	/* gdb machine interface (MI) */
	int mi_issue_cmd(char* cmd, arglist_t* options, arglist_t* parameter, response_hdlr_t resp_hdlr, void* data = 0);
	int mi_parse(char* s);
	int mi_proc_result(result_class_t rclass, unsigned int token, result_t* result);
	int mi_proc_async(result_class_t rclass, unsigned int token, result_t* result);
	int mi_proc_stream(stream_class_t sclass, char* stream);

	/* communication with gdb */
	int sigsend(int sig);
	int read(void* buf, unsigned int nbytes);
	int write(void* buf, unsigned int nbytes);

private:
	/* user command processing */
	int resp_enqueue(unsigned int token, response_hdlr_t hdlr, char* cmdline, void* data);
	int resp_dequeue(unsigned int token);
	mi_cmd_t* resp_query(unsigned int token);


	/* variables */
	// gdb child data
	pty* child_term;
	pid_t child_pid;

	// token used for gdb commands
	unsigned int token;

	// response hash map
	map<unsigned int, mi_cmd_t*> resp_map;
	pthread_mutex_t resp_mutex;
};


#endif

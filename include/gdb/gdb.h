#ifndef GDB_H
#define GDB_H


#include <common/pty.h>
#include <gdb/result.h>
#include <pthread.h>
#include <map>


using namespace std;


/* marcos */
#define GDB_CMD		"/usr/bin/gdb"
#define GDB_ARGS						/* specified as comma-separated list of strings */ \
					"--interpreter=mi", /* enable MI interface */ \
					"-q" 				/* disbale gdb info on start*/


/* types */
typedef int (*response_hdlr_t)(int result_class, result_t* result);


/* class */
class gdb_if{
public:
	/* constructor/desctructor */
	gdb_if();
	~gdb_if();

	/* init gdb interface */
	int init();

	/* gdb machine interface (MI) */
	int mi_cmd_issue(char* cmd, char** options, unsigned int noption, char** parameter, unsigned int nparameter, response_hdlr_t resp_hdlr);

	/* communication with gdb */
	int read(void* buf, unsigned int nbytes);
	int write(void* buf, unsigned int nbytes);

	/* user command processing */
	int resp_enqueue(unsigned int token, response_hdlr_t hdlr);
	int resp_dequeue(unsigned int token);

private:
	/* variables */
	// PTY to gdb child
	pty* child_term;

	// token used for gdb commands
	unsigned int token;

	// response hash map
	map<unsigned int, response_hdlr_t> resp_map;
	pthread_mutex_t resp_mutex;
};


#endif

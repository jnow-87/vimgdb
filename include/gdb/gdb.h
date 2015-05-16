#ifndef GDB_H
#define GDB_H


#include <common/pty.h>
#include <common/list.h>
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


/* class */
class gdbif{
public:
	/* constructor/desctructor */
	gdbif();
	~gdbif();

	/* init gdb interface */
	int init();
	void on_stop(int (*hdlr)(void));
	void on_exit(int (*hdlr)(void));

	/* gdb machine interface (MI) */
	int mi_issue_cmd(char* cmd, gdb_result_class_t ok_mask, int(*process)(gdb_result_t*, void**), void** r, const char* fmt, ...);
	int mi_proc_result(gdb_result_class_t rclass, unsigned int token, gdb_result_t* result);
	int mi_proc_async(gdb_result_class_t rclass, unsigned int token, gdb_result_t* result);
	int mi_proc_stream(gdb_stream_class_t sclass, char* stream);

	/* communication with gdb */
	int read(void* buf, unsigned int nbytes);
	int write(void* buf, unsigned int nbytes);

	int sigsend(int sig);

	/* inferior state */
	bool running();
	bool running(bool state);
	unsigned int threadid();

private:
	/* types */
	typedef struct _response_t{
		gdb_result_class_t volatile rclass;
		gdb_result_t* volatile result;

		struct _response_t * volatile next,
						   * volatile prev;
	} response_t;

	typedef struct _event_hdlr_t{
		int (*hdlr)(void);

		struct _event_hdlr_t *next,
							 *prev;
	} event_hdlr_t;

	/* gdb child data */
	pty* gdb_term;
	pid_t gdb_pid;
	bool volatile is_running;
	unsigned int cur_thread;

	/* gdb communication */
	static void* readline_thread(void* arg);
	static void* event_thread(void* arg);

	unsigned int volatile token;

	response_t resp;
	response_t* volatile event_lst;

	pthread_t read_tid,
			  event_tid;

	pthread_cond_t resp_avail,
				   event_avail;

	pthread_mutex_t resp_mtx,
					event_mtx;

	/* gdb event handling */
	int evt_running(gdb_result_t* result);
	int evt_stopped(gdb_result_t* result);

	event_hdlr_t *stop_hdlr_lst,
				 *exit_hdlr_lst;
};


/* external variables */
extern gdbif* gdb;


#endif

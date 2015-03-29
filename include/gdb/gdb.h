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
	int init(pthread_t main_tid);
	void on_stop(int (*hdlr)(gdbif*));

	/* gdb machine interface (MI) */
	int mi_issue_cmd(char* cmd, gdb_result_class_t ok_mask, void** r, const char* fmt, ...);
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

private:
	/* types */
	typedef struct _response_t{
		gdb_result_class_t rclass;
		gdb_result_t* result;

		struct _response_t *next,
						   *prev;
	} response_t;

	typedef struct _stop_hdlr_t{
		int (*hdlr)(gdbif*);

		struct _stop_hdlr_t *next,
							*prev;
	} stop_hdlr_t;

	/* gdb child data */
	pty* gdb;
	pid_t gdb_pid;
	bool is_running;

	/* gdb communication */
	static void* readline_thread(void* arg);
	static void* event_thread(void* arg);

	unsigned int token;

	response_t resp,
			   *event_lst;

	pthread_t read_tid,
			  event_tid;

	pthread_cond_t resp_avail,
				   event_avail;

	pthread_mutex_t resp_mtx,
					event_mtx;

	/* gdb event handling */
	int evt_running(gdb_result_t* result);
	int evt_stopped(gdb_result_t* result);

	stop_hdlr_t* stop_hdlr;

	/* main thread data */
	pthread_t main_tid;
};


#endif

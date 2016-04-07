#ifndef GUI_VIM_H
#define GUI_VIM_H


#include <common/socket.h>
#include <gui/gui.h>
#include <gui/vim/event.h>
#include <gui/vim/reply.h>
#include <gui/vim/cursor.h>
#include <stdarg.h>
#include <pthread.h>
#include <string>
#include <map>


using namespace std;


class vimui : public gui{
public:
	/* init/destroy */
	vimui();
	~vimui();

	int init();
	void destroy();

	/* user input */
	char* readline();

	/* window functions */
	int win_atomic(int win, bool en);
	int win_create(const char* name, bool oneline = false, unsigned int height = 0);
	int win_getid(const char* name);
	int win_destroy(int win);

	int win_anno_add(int win, int line, const char* sign, const char* color_fg, const char* color_bg);
	int win_anno_delete(int win, int line, const char* sign);

	int win_cursor_set(int win, int line, int col);
	int win_cursor_preserve(int win, bool pc);
	int win_readonly(int win, bool ro);

	void win_print(int win, const char* fmt, ...);
	void win_vprint(int win, const char* fmt, va_list lst);
	void win_clear(int win);

	/* netbeans message handling */
	int proc_reply(int seq_num, vim_reply_t* r);
	int proc_event(int buf_id, vim_event_t* e);

private:
	/* types */
	typedef enum{
		FCT = 1,
		CMD,
	} action_t;

	typedef struct{
		int id;
		char* name;
		unsigned int len;
		bool readonly,
			 preserve;

		vim_cursor_t* cursor;

		map<string, int> annos;
		map<string, int> anno_types;
	} buffer_t;

	/* prototypes */
	int atomic(int win, bool en, bool apply);
	int action(action_t type, const char* action, int buf_id, vim_reply_t** reply, const char* fmt, ...);
	static void* readline_thread(void* arg);

	/* data */
	// vim
	char* cwd;
	bool volatile in_atomic;
	pthread_mutex_t buf_mtx;
	map<string, buffer_t*> bufname_map;
	map<int, buffer_t*> bufid_map;

	// netbeans connection
	socket *nbserver,
		   *nbclient;

	// user input
	pthread_t read_tid;
	pthread_mutex_t event_mtx;
	pthread_cond_t event_avail;
	vim_event_t* volatile event_lst;

	// output
	char* ostr;
	unsigned int ostr_len;
	pthread_mutex_t ui_mtx;

	// reply handling
	pthread_cond_t reply_avail;
	pthread_mutex_t reply_mtx;
	int volatile seq_num;
	vim_reply_t* reply;
};


#endif // GUI_VIM_H

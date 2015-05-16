#ifndef GUI_VIM_H
#define GUI_VIM_H


#include <common/socket.h>
#include <gui/gui.h>
#include <gui/vim/event.h>
#include <gui/vim/result.h>
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
	int atomic(bool state);
	int win_create(const char* name, bool oneline = false, unsigned int height = 0);
	int win_getid(const char* name);
	int win_destroy(int win);

	int win_anno_add(int win, int line, const char* sign, const char* color_fg, const char* color_bg);
	int win_anno_delete(int win, int line, const char* sign);

	int win_cursor_set(int win, int line);

	void win_print(int win, const char* fmt, ...);
	void win_vprint(int win, const char* fmt, va_list lst);
	void win_clear(int win);

	/* netbeans message handling */
	int reply(int seq_num, vim_result_t* rlst);
	int event(int buf_id, int seq_num, vim_event_id_t evt_id, vim_result_t* rlst);

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

		map<string, int> annos;
		map<string, int> anno_types;
	} buffer_t;

	typedef struct _response_t{
		int seq_num,
			buf_id;

		vim_event_id_t volatile evt_id;
		vim_result_t* volatile result;

		struct _response_t * volatile next,
						   * volatile prev;
	} response_t;

	/* prototypes */
	int atomic(bool en, bool apply);
	int action(action_t type, const char* action, int buf_id, int (*process)(vim_result_t*, void*), void* result, const char* fmt, ...);
	static void* readline_thread(void* arg);

	/* data */
	// vim
	char* cwd;
	bool volatile cursor_update;
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
	response_t* volatile event_lst;

	// output
	char* ostr;
	unsigned int ostr_len;
	pthread_mutex_t ui_mtx;

	// response handling
	pthread_cond_t resp_avail;
	pthread_mutex_t resp_mtx;
	response_t resp;
};


#endif // GUI_VIM_H

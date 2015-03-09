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
	int win_create(const char* name, bool oneline = false, unsigned int height = 0);
	int win_getid(const char* name);
	int win_destroy(int win_id);

	void win_print(int win_id, const char* fmt, ...);
	void win_vprint(int win_id, const char* fmt, va_list lst);
	void win_clear(int win_id);

	/* netbeans message handling */
	int reply(int seq_num, vim_result_t* rlst);
	int event(int buf_id, int seq_num, const vim_event_t* evt, vim_result_t* rlst);

private:
	/* types */
	typedef enum{
		FCT = 1,
		CMD,
	} action_t;

	typedef struct _response_t{
		int seq_num,
			buf_id;

		vim_event_id_t evt_id;
		vim_result_t* result;

		struct _response_t *next,
						   *prev;
	} response_t;

	/* netbeans */
	int action(action_t type, const char* action, int buf_id, vim_result_t** result, const char* fmt, ...);
	static void* readline_thread(void* arg);

	/* vim data */
	int bufid;
	map<string, int> bufid_map;
	volatile int seq_num;
	char* cwd;

	/* vim netbeans connection */
	socket *nbserver,
		   *nbclient;

	/* user input */
	pthread_t read_tid;
	pthread_cond_t event_avail;
	response_t* event_lst;

	/* output */
	char* ostr;
	unsigned int ostr_len;
	pthread_mutex_t ui_mtx;

	/* response handling */
	pthread_cond_t resp_avail;
	pthread_mutex_t resp_mtx;
	volatile response_t resp;
};


#endif // GUI_VIM_H

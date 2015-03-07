#ifndef GUI_VIM_H
#define GUI_VIM_H


#include <common/socket.h>
#include <gui/gui.h>
#include <gui/vim/event.h>
#include <gui/vim/result.h>
#include <stdarg.h>
#include <pthread.h>


typedef struct{
	int seq_num,
		buf_id;

	vim_event_id_t evt_id;
	vim_result_t* result;
} vim_response_t;


class vimui : public gui{
public:
	vimui();
	~vimui();

	int init();
	void destroy();

	char* readline();
	int readline_thread();

	int reply(int seq_num, vim_result_t* rlst);
	int event(int buf_id, int seq_num, const vim_event_t* evt, vim_result_t* rlst);

private:
	/* types */
	typedef enum{
		FCT = 1,
		CMD,
	} action_t;

	/* protoypes */
	int win_create(const char* title = "", bool oneline = false, unsigned int height = 0);
	int win_destroy(int win_id);
	void win_write(int win_id, const char* fmt, ...);
	void win_vwrite(int win_id, const char* fmt, va_list lst);
	void win_clear(int win_id);

	int action(action_t type, const char* action, int buf_id, vim_result_t** result, const char* fmt, ...);

	/* vim data */
	int max_buf,
		nbuf;

	bool* buf_ids;
	volatile int seq_num;
	char* cwd;

	/* vim netbeans connection */
	socket *nbserver,
		   *nbclient;

	/* user input */
	pthread_t read_tid;
	pthread_cond_t istr_avail;

	/* output */
	char* ostr;
	unsigned int ostr_len;
	pthread_mutex_t ui_mtx;

	/* response handling */
	pthread_cond_t resp_avail;
	pthread_mutex_t resp_mtx;
	volatile vim_response_t resp;
};


#endif // GUI_VIM_H

#ifndef GUI_VIM_H
#define GUI_VIM_H


#include <common/socket.h>
#include <gui/gui.h>
#include <gui/vim/event.h>
#include <stdarg.h>


class vimui : public gui{
public:
	vimui();
	~vimui();

	int init();
	void destroy();

	char* readline();

private:
	/* types */
	typedef enum{
		FCT = 1,
		CMD,
	} vim_action_t;

	/* protoypes */
	int win_create(const char* title = "", bool oneline = false, unsigned int height = 0);
	int win_destroy(int win_id);
	void win_write(int win_id, const char* fmt, ...);
	void win_vwrite(int win_id, const char* fmt, va_list lst);
	void win_clear(int win_id);

	int vim_action(vim_action_t type, const char* action, int buf_id, const char* fmt, ...);

	/* vim data */
	int max_buf,
		nbuf;

	int* buf_ids;
	int seq_num;

	/* vim netbeans connection */
	socket *nbserver, *nbclient;

	/* user input */
	char* line;
	unsigned int line_len;
};


#endif // GUI_VIM_H

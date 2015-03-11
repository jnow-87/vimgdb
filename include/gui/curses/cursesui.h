#ifndef GUI_CURSES_H
#define GUI_CURSES_H


#include <common/tty.h>
#include <gui/gui.h>
#include <curses.h>
#include <pthread.h>
#include <list>


struct window_t{
	WINDOW *win, *frame;
	char* title;
	bool oneline;
	unsigned int height;
};


class cursesui : public gui{
public:
	/* init/destroy */
	cursesui();
	~cursesui();

	int init();
	void destroy();

	/* user input */
	char* readline();

	/* window functions */
	int win_create(const char* name, bool oneline = false, unsigned int height = 0);
	int win_getid(const char* name);
	int win_destroy(int win_id);

	int win_anno_add(int win, int line, const char* sign, const char* color_fg, const char* color_bg);
	int win_anno_delete(int win, int line);

	void win_print(int win_id, const char* fmt, ...);
	void win_vprint(int win_id, const char* fmt, va_list lst);
	void win_clear(int win_id);
	void win_clrline(int win_id);

private:
	int win_resize();

	/* window information */
	int max_win,
		nwin;

	window_t** windows;

	/* user input */
	char* line;
	unsigned int line_len;
	int user_win_id;
	tty* term;

	/* threading */
	pthread_mutex_t mutex;
};


#endif // GUI_CURSES_H

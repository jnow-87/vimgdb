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
	cursesui();
	~cursesui();

	char* readline();

private:
	int win_create(const char* title = "", bool oneline = false, unsigned int height = 0);
	int win_destroy(int win_id);
	void win_write(int win_id, const char* fmt, ...);
	void win_vwrite(int win_id, const char* fmt, va_list lst);
	void win_clear(int win_id);
	void win_clrline(int win_id);

	int win_resize();

	/* window information */
	int max_win,
		nwin;

	window_t** windows;

	/* user input */
	char* line;
	unsigned int line_len;
	tty* term;

	/* threading */
	pthread_mutex_t mutex;
};


#endif // GUI_CURSES_H

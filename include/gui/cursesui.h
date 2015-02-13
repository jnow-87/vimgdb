#ifndef GUI_CURSES_H
#define GUI_CURSES_H


#include <gui/gui.h>
#include <curses.h>
#include <pthread.h>
#include <list>


using namespace std;


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


private:
	int win_create(const char* title = "", bool oneline = false, unsigned int height = 0);
	int win_destroy(int win_id);
	void win_write(int win_id, const char* fmt, ...);
	void win_vwrite(int win_id, const char* fmt, va_list lst);
	void win_clear(int win_id);
	void win_clrline(int win_id);

	int win_resize();

	int max_win,
		nwin;
	pthread_mutex_t mutex;

	window_t** windows;
};


#endif // GUI_CURSES_H

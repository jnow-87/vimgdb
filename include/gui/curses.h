#ifndef GUI_CURSES_H
#define GUI_CURSES_H


#include <gui/gui.h>
#include <curses.h>
#include <list>


using namespace std;


struct window_t{
	WINDOW *win, *frame;
	char* title;
	bool oneline;
};


class curses : public gui{
public:
	curses();
	~curses();


private:
	int win_create(const char* title = "", bool oneline = false);
	int win_destroy(int win_id);
	void win_write(int win_id, const char* fmt, ...);
	void win_vwrite(int win_id, const char* fmt, va_list lst);

	int win_resize();

	int max_win,
		nwin;

	window_t** windows;
};


#endif // GUI_CURSES_H

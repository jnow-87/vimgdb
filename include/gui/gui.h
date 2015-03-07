#ifndef GUI_H
#define GUI_H


#include <stdarg.h>
#include <stdio.h>


/* macros */
#define WIN_INIT(_title, _oneline, _height) \
	{ .id = 0, .title = _title, .oneline = _oneline , .height = _height }


/* types */
struct win_cfg_t{
	int id;
	const char* title;
	bool oneline;
	unsigned int height;
};

// XXX the order of those ids has
// to match the order of the entries
// in wins[]
enum win_id_t{
	WIN_BREAK = 0,
	WIN_INFERIOR,
	WIN_GDBLOG,
	WIN_USERLOG,
#ifdef GUI_CURSES
	WIN_DEBUGLOG,
#endif
};


/* class */
class gui{
public:
	void print(win_id_t win, const char* fmt, ...);
	void vprint(win_id_t win, const char* fmt, va_list lst);
	void clear(win_id_t win);
	void clearline(win_id_t win);

	virtual int init() = 0;
	virtual void destroy() = 0;
	virtual char* readline() = 0;

protected:
	int base_init();
	void base_destroy();

	static int min_win_height,
			   min_win_width;

private:
	virtual int win_create(const char* title = "", bool oneline = false, unsigned int height = 0) = 0;
	virtual int win_destroy(int win_id) = 0;
	virtual void win_write(int win_id, const char* fmt, ...) = 0;
	virtual void win_vwrite(int win_id, const char* fmt, va_list lst) = 0;
	virtual void win_clear(int win_id) = 0;

	static win_cfg_t wins[];
};


/* global variables */
extern gui* ui;


/* gui implementation header */
#ifdef GUI_CURSES
	#include <gui/curses/cursesui.h>
#elif GUI_VIM
	#include <gui/vim/vimui.h>
#else
	#error "invalid gui defined"
#endif


#endif // GUI_H

#ifndef GUI_H
#define GUI_H


#include <stdarg.h>
#include <pthread.h>


/* class */
class gui{
public:
	/* init/destroy */
	virtual ~gui(){}
	virtual int init() = 0;
	virtual void destroy() = 0;

	/* user input */
	virtual char *readline() = 0;

	/* window functions */
	virtual int win_atomic(int win, bool en) = 0;
	virtual int win_create(const char *name, bool oneline = false, unsigned int height = 0) = 0;
	virtual int win_getid(const char *name) = 0;
	virtual int win_destroy(int win_id) = 0;

	virtual int win_anno_add(int win, int line, const char *sign, const char *color_fg, const char *color_bg) = 0;
	virtual int win_anno_delete(int win, int line, const char *sign) = 0;

	virtual int win_cursor_set(int win, int line, int col = 0) = 0;
	virtual int win_cursor_preserve(int win, bool pc) = 0;
	virtual int win_readonly(int win, bool ro) = 0;

	virtual void win_print(int win, const char *fmt, ...) = 0;
	virtual void win_vprint(int win, const char *fmt, va_list lst) = 0;
	virtual void win_clear(int win) = 0;
};


/* global variables */
extern gui *ui;


#endif // GUI_H

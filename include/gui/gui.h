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


/* class */
class gui{
public:
	int init();
	void destroy();

	void applog_print(const char* fmt, ...);
	void applog_vprint(const char* fmt, va_list lst);

	void userlog_print(const char* fmt, ...);
	void userlog_vprint(const char* fmt, va_list lst);

	void gdblog_print(const char* fmt, ...);

	void break_print(const char* fmt, ...);
	void break_clear();

	void cmd_print(const char* fmt, ...);
	void cmd_clrline();

	void inferior_print(const char* fmt, ...);

protected:
	static int min_win_height,
			   min_win_width;

private:
	virtual int win_create(const char* title = "", bool oneline = false, unsigned int height = 0) = 0;
	virtual int win_destroy(int win_id) = 0;
	virtual void win_write(int win_id, const char* fmt, ...) = 0;
	virtual void win_vwrite(int win_id, const char* fmt, va_list lst) = 0;
	virtual void win_clear(int win_id) = 0;
	virtual void win_clrline(int win_id) = 0;

	static win_cfg_t wins[];

	// XXX the order of those ids has
	// to match the order of the entries
	// in wins[]
	enum win_id_t{
		WIN_BRK = 0,
		WIN_INFERIOR,
		WIN_GDB_LOG,
		WIN_APPLOG,
		WIN_USERLOG,
		WIN_CMD,
	};
};


/* global variables */
extern gui* ui;


#endif // GUI_H

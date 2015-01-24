#ifndef GUI_H
#define GUI_H


#include <stdarg.h>
#include <stdio.h>


/* macros */
#define WIN_INIT(_title, _oneline) \
	{ .id = 0, .title = _title, .oneline = _oneline }


/* types */
struct win_cfg_t{
	int id;
	const char* title;
	bool oneline;
};


/* class */
class gui{
public:
	int init();
	void destroy();

	void win_log(const char* fmt, ...);
	void win_gdb_log(const char* fmt, ...);
	void win_break();
	void win_cmd(const char* fmt, ...);


protected:
	static int min_win_height,
			   min_win_width;

private:
	virtual int win_create(const char* title = "", bool oneline = false) = 0;
	virtual int win_destroy(int win_id) = 0;
	virtual void win_write(int win_id, const char* fmt, ...) = 0;
	virtual void win_vwrite(int win_id, const char* fmt, va_list lst) = 0;

	static win_cfg_t wins[];

	// XXX the order of those ids has
	// to match the order of the entries
	// in wins[]
	enum win_id_t{
		WIN_BRK = 0,
		WIN_DUMMY,
		WIN_GDB_LOG,
		WIN_LOG,
		WIN_CMD,
	};
};


#endif // GUI_H

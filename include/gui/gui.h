#ifndef GUI_H
#define GUI_H


#include <stdarg.h>


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
	virtual int win_create(const char* title = "") = 0;
	virtual int win_destroy(int win_id) = 0;
	virtual void win_write(int win_id, const char* fmt, ...) = 0;
	virtual void win_vwrite(int win_id, const char* fmt, va_list lst) = 0;

	/* window ids */
	int* wins;
	static int nwin;
	static const char* win_title[];

	enum win_id_t{
		WIN_LOG = 0,
		WIN_GDB_LOG,
		WIN_CMD,
		WIN_BRK,
	};
};


#endif // GUI_H

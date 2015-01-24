#include <gui/gui.h>


/* static members */
int gui::min_win_height = 3;
int gui::min_win_width = 30;

win_cfg_t gui::wins[] = {
	WIN_INIT("breakpoints", false),
	WIN_INIT("dummy", false),
	WIN_INIT("gdb-log", true),
	WIN_INIT("log", true),
	WIN_INIT("command-line", true),
	{ .id = -1 }	/* dummy entry, marking the end */
};


/* member functions */
int gui::init(){
	unsigned int i;


	i = 0;
	while(wins[i].id != -1){
		wins[i].id = win_create(wins[i].title, wins[i].oneline);
		if(wins[i].id <= 0)
			return -1;
		i++;
	}

	return 0;
}

void gui::destroy(){
	unsigned int i;


	i = 0;
	while(wins[i].id != -1){
		if(wins[i].id > 0)
			win_destroy(wins[i].id);
		i++;
	}
}

void gui::win_log(const char* fmt, ...){
	va_list lst;


	va_start(lst, fmt);
	win_vwrite(wins[WIN_LOG].id, fmt, lst);
	va_end(lst);
}

void gui::win_gdb_log(const char* fmt, ...){
	va_list lst;


	va_start(lst, fmt);
	win_vwrite(wins[WIN_GDB_LOG].id, fmt, lst);
	va_end(lst);
}

void gui::win_break(){
}

void gui::win_cmd(const char* fmt, ...){
	va_list lst;


	va_start(lst, fmt);
	win_vwrite(wins[WIN_CMD].id, fmt, lst);
	va_end(lst);
}

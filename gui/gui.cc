#include <gui/gui.h>


/* global variables */
gui* ui = 0;

/* static members */
int gui::min_win_height = 3;
int gui::min_win_width = 30;

win_cfg_t gui::wins[] = {
	WIN_INIT("breakpoints", false, 0),
	WIN_INIT("inferior", false, 0),
	WIN_INIT("gdb-log", true, 0),
	WIN_INIT("log", true, 0),
	WIN_INIT("command-line", true, 3),
	{ .id = -1 }	/* dummy entry, marking the end */
};


/* member functions */
int gui::init(){
	unsigned int i;


	i = 0;
	while(wins[i].id != -1){
		wins[i].id = win_create(wins[i].title, wins[i].oneline, wins[i].height);
		if(wins[i].id < 0)
			return -1;
		i++;
	}

	return 0;
}

void gui::destroy(){
	unsigned int i;


	i = 0;
	while(wins[i].id != -1){
		if(wins[i].id >= 0)
			win_destroy(wins[i].id);
		i++;
	}
}

void gui::log_print(const char* fmt, ...){
	va_list lst;


	va_start(lst, fmt);
	win_vwrite(wins[WIN_LOG].id, fmt, lst);
	va_end(lst);
}

void gui::log_vprint(const char* fmt, va_list lst){
	win_vwrite(wins[WIN_LOG].id, fmt, lst);
}

void gui::gdblog_print(const char* fmt, ...){
	va_list lst;


	va_start(lst, fmt);
	win_vwrite(wins[WIN_GDB_LOG].id, fmt, lst);
	va_end(lst);
}

void gui::break_print(){
}

void gui::cmd_print(const char* fmt, ...){
	va_list lst;


	va_start(lst, fmt);
	win_vwrite(wins[WIN_CMD].id, fmt, lst);
	va_end(lst);
}

void gui::cmd_clrline(){
	win_clrline(wins[WIN_CMD].id);
}

void gui::inferior_print(const char* fmt, ...){
	va_list lst;


	va_start(lst, fmt);
	win_vwrite(wins[WIN_INFERIOR].id, fmt, lst);
	va_end(lst);
}

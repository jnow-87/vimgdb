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
	WIN_INIT("user-log", true, 0),
#ifdef GUI_CURSES
	WIN_INIT("debug-log", true, 0),
#endif
	{ .id = -1 }	/* dummy entry, marking the end */
};


/* member functions */
void gui::print(win_id_t win, const char* fmt, ...){
	va_list lst;


	va_start(lst, fmt);
	win_vwrite(wins[win].id, fmt, lst);
	va_end(lst);
}

void gui::vprint(win_id_t win, const char* fmt, va_list lst){
	win_vwrite(wins[win].id, fmt, lst);
}

void gui::clear(win_id_t win){
	win_clear(wins[win].id);
}

void gui::clearline(win_id_t win){
	win_clrline(wins[win].id);
}

int gui::base_init(){
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

void gui::base_destroy(){
	unsigned int i;


	i = 0;
	while(wins[i].id != -1){
		if(wins[i].id >= 0)
			win_destroy(wins[i].id);
		i++;
	}
}



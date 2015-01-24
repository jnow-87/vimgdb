#include <gui/gui.h>


int gui::min_win_height = 3;
int gui::min_win_width = 30;

int gui::nwin = 4;

const char* gui::win_title[] = {
	"log",
	"gdb-log",
	"cmd",
	"breakpoints",
};

int gui::init(){
	unsigned int i;


	wins = new int[nwin];

	for(i=0; i<4; i++){
		wins[i] = win_create(win_title[i]);
		if(wins[i] < 0)
			return -1;
	}

	return 0;
}

void gui::destroy(){
	unsigned int i;


	for(i=0; i<4; i++){
		if(wins[i] > 0)
			win_destroy(wins[i]);
	}

	delete wins;
}

void gui::win_log(const char* fmt, ...){
	va_list lst;


	va_start(lst, fmt);
	win_vwrite(WIN_LOG, fmt, lst);
	va_end(lst);
}

void gui::win_gdb_log(const char* fmt, ...){
	va_list lst;


	va_start(lst, fmt);
	win_vwrite(WIN_GDB_LOG, fmt, lst);
	va_end(lst);}

void gui::win_break(){
}

void gui::win_cmd(const char* fmt, ...){
	va_list lst;


	va_start(lst, fmt);
	win_vwrite(WIN_CMD, fmt, lst);
	va_end(lst);
}

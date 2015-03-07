#include <common/string.h>
#include <gui/vim/vimui.h>
#include <stdlib.h>
#include <string.h>


vimui::vimui(){
}

vimui::~vimui(){
	free(buf_ids);
	free(line);

	delete nbclient;
	delete nbserver;
}

int vimui::init(){
	return base_init();
}

void vimui::destroy(){
	base_destroy();
}

char* vimui::readline(){
	return 0;
}

int vimui::win_create(const char* title, bool oneline, unsigned int height){
	return -1;
}

int vimui::win_destroy(int win_id){
	return -1;
}

void vimui::win_write(int win_id, const char* fmt, ...){
}

void vimui::win_vwrite(int win_id, const char* fmt, va_list lst){
}

void vimui::win_clear(int win_id){
}

int vimui::vim_action(vim_action_t type, const char* action, int buf_id, const char* fmt, ...){
	return -1;
}

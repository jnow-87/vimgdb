#include <gui/vim/cursor.h>


int result_to_cursor(vim_result_t* r, void* _cur){
	vim_cursor_t* cur;


	cur = (vim_cursor_t*)_cur;

	cur->bufid = r->num;
	cur->line = r->next->num;
	cur->column = r->next->next->num;

	return 0;
}

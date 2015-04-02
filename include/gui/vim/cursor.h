#ifndef VIM_CURSOR_H
#define VIM_CURSOR_H


#include <gui/vim/result.h>


/* types */
typedef struct{
	int bufid,
		line,
		column;
} vim_cursor_t;


/* prototypes */
int result_to_cursor(vim_result_t* r, void* cur);


#endif // VIM_CURSOR_H

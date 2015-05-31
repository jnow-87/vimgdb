#ifndef VIM_CURSOR_H
#define VIM_CURSOR_H


#include <gui/vim/reply.h>


/* class */
class vim_cursor_t : public vim_reply_t{
public:
	int bufid,
		line,
		column;
};


#endif // VIM_CURSOR_H

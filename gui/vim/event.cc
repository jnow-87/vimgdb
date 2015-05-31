#include <gui/vim/event.h>


vim_event_t::vim_event_t(){
	data = 0;
}

vim_event_t::~vim_event_t(){
	delete [] data;
}

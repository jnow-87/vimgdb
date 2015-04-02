#include <gui/vim/length.h>


int result_to_length(vim_result_t* r, void* len){
	*((int*)len) = r->num;
	return 0;
}

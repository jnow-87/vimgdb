#include <common/list.h>
#include <gui/vim/result.h>


vim_result_t* vim_result_create(vim_result_type_t type, void* val0, void* val1){
	vim_result_t* r;


	r = new vim_result_t;

	if(r == 0)
		return 0;

	list_init(r);
	r->type = type;

	if(type == RT_STR)				r->sptr = (char*)val0;
	else if(type == RT_INT)			r->num = *((int*)val0);
	else if(type == RT_LINENUM){
		r->line = *((int*)val0);
		r->column = *((int*)val1);
	}

	return r;
}

vim_result_t* vim_result_free(vim_result_t* lst){
	vim_result_t* r;


	list_for_each(lst, r){
		list_rm(&lst, r);

		if(r->type == RT_STR)
			delete r->sptr;

		delete r;
	}

	return lst;
}

vim_result_t* vim_result_add(vim_result_t* lst, vim_result_t* r){
	if(lst == 0)
		return r;

	list_add_tail(lst, r);
	return lst;
}

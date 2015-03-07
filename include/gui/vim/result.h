#ifndef VIM_RESULT_H
#define VIM_RESULT_H


typedef enum{
	RT_STR = 1,
	RT_INT,
	RT_LINENUM
} vim_result_type_t;

typedef struct _vim_result_t{
	vim_result_type_t type;

	union{
		int num;
		int line,
			column;

		char* sptr;
	};
	
	struct _vim_result_t *next, *prev;
} vim_result_t;


vim_result_t* vim_result_create(vim_result_type_t type, void* val0, void* val1 = 0);
vim_result_t* vim_result_free(vim_result_t* lst);
vim_result_t* vim_result_add(vim_result_t* lst, vim_result_t* r);


#endif // VIM_RESULT_H

#ifndef VIM_FCT_H
#define VIM_FCT_H


typedef enum{
	F_GETCURSOR = 1,
	F_GETLENGTH,
	F_INSERT,
	F_REMOVE,
} vim_fct_id_t;

struct vim_fct_t{
	const char* name;
	vim_fct_id_t id;
};

typedef vim_fct_t vim_fct_t;


#endif // VIM_FCT_H

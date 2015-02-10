#ifndef ARGLIST_H
#define ARGLIST_H


#include <common/list.h>


/* types */
typedef enum{
	T_INT = 1,
	T_STRING,
} arglist_type_t;

typedef struct arglist_t{
	struct arglist_t *next,
					 *prev;

	arglist_type_t type;
	bool quoted;

	union{
		char* sptr;
		int i;
	} value;
} arglist_t;


/* macros */
#define _add_el(lst, el){ \
	if(lst == 0){ \
		list_init(_el); \
		lst = _el; \
	} \
	else \
		list_add_tail(lst, _el); \
}

#define arg_add_int(lst, val, quote){ \
	arglist_t* _el; \
	\
	_el = new arglist_t; \
	(_el)->quoted = quote; \
	(_el)->value.i = val; \
	(_el)->type = T_INT; \
	_add_el(lst, _el); \
}

#define arg_add_string(lst, val, quote){ \
	arglist_t* _el; \
	\
	_el = new arglist_t; \
	(_el)->quoted = quote; \
	(_el)->value.sptr = val; \
	(_el)->type = T_STRING; \
	\
	_add_el(lst, _el); \
}

#define arg_clear(lst){ \
	arglist_t* _el; \
	\
	list_for_each(lst, _el){ \
		list_rm(&(lst), _el); \
		delete _el; \
	} \
}


#endif // ARGLIST_H

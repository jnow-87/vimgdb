#ifndef VIM_CMD_H
#define VIM_CMD_H


#include <gui/vim/event.h>


typedef enum{
	C_CREATE = 1,
	C_CLOSE,
	C_SAVE,
	C_EDITFILE,
	C_PUTBUFFERNUMBER,
	C_SETBUFFERNUMBER,
	C_SETDOT,
	C_SETMODIFIED,
	C_SETREADONLY,
	C_SETTITLE,
	C_SETFULLNAME,
	C_SETVISIBLE,
	C_STARTDOCUMENTLISTEN,
	C_STOPDOCUMENTLISTEN,
	C_INITDONE,
	C_INSERTDONE,
	C_SAVEDONE,
	C_ADDANNO,
	C_DEFINEANNOTYPE,
	C_REMOVEANNO,
	C_GUARD,
	C_UNGUARD,
	C_ATOMICSTART,
	C_ATOMICEND,
} vim_cmd_id_t;

struct vim_cmd_t{
	const char *name;
	vim_cmd_id_t id;
};

typedef vim_cmd_t vim_cmd_t;


#endif // VIM_CMD_H

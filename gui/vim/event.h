#ifndef VIM_EVENT_H
#define VIM_EVENT_H


typedef enum{
	E_DISCONNECT = 1,
	E_STARTUPDONE,
	E_KILLED,
	E_FILEOPENED,
	E_INSERT,
	E_REMOVE,
	E_KEYCOMMAND,
	E_KEYATPOS,
	E_NEWDOTANDMARK,
	E_ACCEPT,
	E_AUTH,
	E_DETACH,
	E_VERSION,
} vim_event_id_t;

struct vim_event_t{
	const char* name;
	vim_event_id_t id;
};

typedef vim_event_t vim_event_t;


#endif // VIM_EVENT_H

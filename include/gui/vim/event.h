#ifndef VIM_EVENT_H
#define VIM_EVENT_H


typedef enum{
	E_NONE = 0,
	E_DISCONNECT = 0x1,
	E_STARTUPDONE = 0x2,
	E_KILLED = 0x4,
	E_FILEOPENED = 0x8,
	E_INSERT = 0x10,
	E_REMOVE = 0x20,
	E_KEYCOMMAND = 0x40,
	E_KEYATPOS = 0x80,
	E_NEWDOTANDMARK = 0x100,
	E_ACCEPT = 0x200,
	E_AUTH = 0x400,
	E_DETACH = 0x800,
	E_VERSION = 0x1000,
} vim_event_id_t;

struct vim_event_t{
	const char* name;
	vim_event_id_t id;
};

typedef vim_event_t vim_event_t;


#endif // VIM_EVENT_H

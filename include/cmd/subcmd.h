#ifndef SUBCMD_H
#define SUBCMD_H


/* types */
typedef enum{
	BIN = 1,
	SYM,
	ARGS,
	ADD,
	DELETE,
	ENABLE,
	DISABLE,
} subcmd_id_t;

struct subcmd_t{
	const char* name;
	subcmd_id_t id;
};

typedef subcmd_t subcmd_t;


#endif // SUBCMD_H

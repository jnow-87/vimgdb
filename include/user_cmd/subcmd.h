#ifndef SUBCMD_H
#define SUBCMD_H


/* types */
typedef enum{
	BIN = 1,
	SYM,
	ARGS,
	TTY,
	ADD,
	DELETE,
	ENABLE,
	DISABLE,
	RUN,
	CONTINUE,
	NEXT,
	STEP,
	RETURN,
	BREAK,
	JUMP,
	GOTO,
} user_subcmd_id_t;

struct user_subcmd_t{
	const char* name;
	user_subcmd_id_t id;
};

typedef user_subcmd_t user_subcmd_t;


#endif // SUBCMD_H

#ifndef CMD_H
#define CMD_H


#include <user_cmd/exec.h>
#include <user_cmd/inferior.h>
#include <user_cmd/break.h>
#include <user_cmd/variable.h>
#include <user_cmd/callstack.h>
#include <user_cmd/register.h>
#include <user_cmd/memory.h>
#include <user_cmd/evaluate.h>
#include <user_cmd/help.h>
#include <user_cmd/test.h>


/* types */
struct user_cmd_t{
	const char* name;
	int (*exec)(int argc, char** argv);
	void (*help)(int argc, char** argv);
	const char* help_msg;
};

typedef user_cmd_t user_cmd_t;


/* prototypes */
int cmd_exec(char* line);


#endif // CMD_H

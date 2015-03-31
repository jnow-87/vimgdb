#ifndef CMD_H
#define CMD_H


#include <gdb/gdb.h>
#include <user_cmd/exec.h>
#include <user_cmd/inferior.h>
#include <user_cmd/break.h>
#include <user_cmd/variable.h>
#include <user_cmd/help.h>
#include <user_cmd/test.h>


/* types */
struct user_cmd_t{
	const char* name;
	int (*exec)(gdbif* gdb, int argc, char** argv);
	void (*help)(int argc, char** argv);
	const char* help_msg;
};

typedef user_cmd_t user_cmd_t;


/* prototypes */
int cmd_exec(char* cmdline, gdbif* gdb);


#endif // CMD_H

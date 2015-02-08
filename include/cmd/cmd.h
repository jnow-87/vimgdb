#ifndef CMD_H
#define CMD_H


#include <gdb/gdb.h>


/* types */
struct cmd_t{
	const char* name;
	int (*exec)(gdb_if* gdb, int argc, char** argv);
	void (*help)(char* cmd);
	const char* help_msg;
};

typedef cmd_t cmd_t;


/* prototypes */
// general command processing
int cmd_exec(char* cmdline, gdb_if* gdb);

// commands
// inferior
int cmd_inferior_exec(gdb_if* gdb, int argc, char** argv);
int cmd_inferior_resp(result_class_t rclass, result_t* result, char* cmdline, void* data);
void cmd_inferior_help(char* cmd);

// break
int cmd_break_exec(gdb_if* gdb, int argc, char** argv);
int cmd_break_resp(result_class_t rclass, result_t* result, char* cmdline, void* data);
void cmd_break_help(char* cmd);

// help
int cmd_help_exec(gdb_if* gdb, int argc, char** argv);

// test
int cmd_test_exec(gdb_if* gdb, int argc, char** argv);
void cmd_test_help(char* cmd);


#endif // CMD_H

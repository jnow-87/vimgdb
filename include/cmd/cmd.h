#ifndef CMD_H
#define CMD_H


#include <gdb/gdb.h>


/* types */
struct cmd_t{
	const char* name;
	int (*exec)(gdb_if* gdb, int argc, char** argv);
	void (*help)(int argc, char** argv);
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
void cmd_inferior_help(int argc, char** argv);

// break
int cmd_break_exec(gdb_if* gdb, int argc, char** argv);
int cmd_break_resp(result_class_t rclass, result_t* result, char* cmdline, void* data);
void cmd_break_help(int argc, char** argv);

// exec
int cmd_exec_exec(gdb_if* gdb, int argc, char** argv);
int cmd_exec_resp(result_class_t rclass, result_t* result, char* cmdline, void* data);
void cmd_exec_help(int argc, char** argv);

// help
int cmd_help_exec(gdb_if* gdb, int argc, char** argv);

// test
int cmd_test_exec(gdb_if* gdb, int argc, char** argv);
void cmd_test_help(int argc, char** argv);


#endif // CMD_H

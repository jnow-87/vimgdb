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
int cmd_file_exec(gdb_if* gdb, int argc, char** argv);
int cmd_file_resp(int result_class, result_t* result);
void cmd_file_help(char* cmd);

int cmd_help_exec(gdb_if* gdb, int argc, char** argv);

int cmd_test_exec(gdb_if* gdb, int argc, char** argv);
void cmd_test_help(char* cmd);


#endif // CMD_H

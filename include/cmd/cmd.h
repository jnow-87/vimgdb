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
// file
int cmd_file_exec(gdb_if* gdb, int argc, char** argv);
int cmd_file_resp(result_class_t rclass, result_t* result, char* cmdline);
void cmd_file_help(char* cmd);

// inftty
int cmd_inftty_exec(gdb_if* gdb, int argc, char** argv);
int cmd_inftty_resp(result_class_t rclass, result_t* result, char* cmdline);
void cmd_inftty_help(char* cmd);

// help
int cmd_help_exec(gdb_if* gdb, int argc, char** argv);

// test
int cmd_test_exec(gdb_if* gdb, int argc, char** argv);
void cmd_test_help(char* cmd);


#endif // CMD_H

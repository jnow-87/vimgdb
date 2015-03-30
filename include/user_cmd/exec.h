#ifndef USER_EXEC_H
#define USER_EXEC_H


#include <gdb/gdb.h>


int cmd_exec_exec(gdbif* gdb, int argc, char** argv);
void cmd_exec_help(int argc, char** argv);


#endif // USER_EXEC_H

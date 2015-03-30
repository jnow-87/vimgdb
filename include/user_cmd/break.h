#ifndef USER_BREAK_H
#define USER_BREAK_H


#include <gdb/gdb.h>


int cmd_break_exec(gdbif* gdb, int argc, char** argv);
void cmd_break_help(int argc, char** argv);


#endif // USER_BREAK_H

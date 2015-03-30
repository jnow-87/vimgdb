#ifndef USER_INFERIOR_H
#define USER_INFERIOR_H


#include <gdb/gdb.h>


int cmd_inferior_exec(gdbif* gdb, int argc, char** argv);
void cmd_inferior_help(int argc, char** argv);


#endif // USER_INFERIOR_H

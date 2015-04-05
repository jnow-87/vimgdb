#ifndef USER_VARIABLE_H
#define USER_VARIABLE_H


#include <gdb/gdb.h>


/* prototypes */
int cmd_var_exec(gdbif* gdb, int argc, char** argv);
void cmd_var_help(int argc, char** argv);
int cmd_var_update(gdbif* gdb);


#endif // USER_VARIABLE_H
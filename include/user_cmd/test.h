#ifndef USER_TEST_H
#define USER_TEST_H


#include <gdb/gdb.h>


int cmd_test_exec(gdbif* gdb, int argc, char** argv);
void cmd_test_help(int argc, char** argv);


#endif // USER_TEST_H

#ifndef USER_CALLSTACK_H
#define USER_CALLSTACK_H


bool cmd_callstack_exec(int argc, char **argv);
void cmd_callstack_cleanup();
void cmd_callstack_help(int argc, char **argv);
int cmd_callstack_update();
int cmd_callstack_print();


#endif // USER_CALLSTACK_H

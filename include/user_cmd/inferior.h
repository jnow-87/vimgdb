#ifndef USER_INFERIOR_H
#define USER_INFERIOR_H


int cmd_inferior_exec(int argc, char **argv);
void cmd_inferior_cleanup();
void cmd_inferior_help(int argc, char **argv);


#endif // USER_INFERIOR_H

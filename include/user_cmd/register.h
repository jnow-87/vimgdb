#ifndef USER_REGISTER_H
#define USER_REGISTER_H


/* prototypes */
int cmd_register_init();
bool cmd_register_exec(int argc, char **argv);
void cmd_register_cleanup();
void cmd_register_help(int argc, char **argv);
int cmd_register_print();


#endif // USER_REGISTER_H

#ifndef GDB_H
#define GDB_H


#include <common/pty.h>


/* marcos */
#define GDB_CMD		"/usr/bin/gdb"
#define GDB_ARGS						/* specified as comma-separated list of strings */ \
					"--interpreter=mi", /* enable MI interface */ \
					"-q" 				/* disbale gdb info on start*/


/* class */
class gdb_if{
public:
	/* constructor/desctructor */
	gdb_if();
	~gdb_if();

	/* init gdb interface */
	int init();

	/* communication with gdb */
	int read(void* buf, unsigned int nbytes);
	int write(void* buf, unsigned int nbytes);

	/* user command processing */
	int exec_user_cmd(char* cmdline);

	/* user commands */
	static int cmd_test(gdb_if* gdb, int argc, char** argv);
	static int cmd_help(gdb_if* gdb, int argc, char** argv);

private:
	pty* child_term;	// PTY to gdb child
};


/* types */
struct gdb_user_cmd_t{
	const char* name;

	int (*callback)(gdb_if* gdb, int argc, char** argv);

	const char* help_msg;
};

typedef gdb_user_cmd_t gdb_user_cmd_t;


#endif

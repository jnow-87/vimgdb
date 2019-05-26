#ifndef CMD_H
#define CMD_H


#include <user_cmd/exec.h>
#include <user_cmd/inferior.h>
#include <user_cmd/break.h>
#include <user_cmd/variable.h>
#include <user_cmd/callstack.h>
#include <user_cmd/register.h>
#include <user_cmd/memory.h>
#include <user_cmd/evaluate.h>
#include <user_cmd/per.h>
#include <user_cmd/ui.h>
#include <user_cmd/help.h>
#include <user_cmd/test.h>


/* types */
struct user_cmd_t{
	const char *name;

	/**
	 * \brief	callback to execute a command
	 *
	 * \param	argc	number of arguments in argv
	 * \param	argv	list of arguments
	 *
	 * \return	true	the command causes an asynchronous update of the
	 * 					inferior state, thus requiring the exec-calling
	 * 					function to wait for that update
	 *
	 * 			false	the command odes not cause an asynchronous update
	 * 					of the inferior state
	 */
	bool (*exec)(int argc, char **argv);

	/**
	 * \brief	callback to handle cleanup on application shutdown
	 */
	void (*cleanup)();

	/**
	 * \brief	call to request help on a command
	 *
	 * \param	argc	number of arguments in argv
	 * \param	argv	list of arguments
	 */
	void (*help)(int argc, char **argv);


	const char *help_msg;	/**< short help message */
};

typedef user_cmd_t user_cmd_t;


/* prototypes */
void cmd_exec(char *line);


#endif // CMD_H

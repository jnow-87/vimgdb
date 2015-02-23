#include <common/log.h>
#include <common/pty.h>
#include <common/string.h>
#include <gdb/gdb.h>
#include <gdb/lexer.lex.h>
#include <gdb/parser.tab.h>
#include <user_cmd/cmd.hash.h>
#include <user_cmd/subcmd.hash.h>
#include <string.h>
#include <signal.h>
#include <math.h>


/* class definition */
/**
 * \brief	standard constructor
 */
gdbif::gdbif(){
	child_term = 0;
	token = 1;

	pthread_mutex_init(&resp_mtx, 0);
	pthread_cond_init(&resp_avail, 0);
}

/**
 * \brief	standard desctructor
 */
gdbif::~gdbif(){
	// close gdb terminal
	delete this->child_term;
	this->child_term = 0;

	pthread_cond_destroy(&resp_avail);
	pthread_mutex_destroy(&resp_mtx);
}

/**
 * \brief	exec gdb and initialise its controlling terminal
 *
 * \return	0	on success
 * 			-1	on error (check errno)
 */
int gdbif::init(){
	// initialise pseudo terminal
	this->child_term = new pty();

	// fork child process
	child_pid = child_term->fork();

	if(child_pid == 0){
		/* child */
		log::cleanup();

		return execl(GDB_CMD, GDB_CMD, GDB_ARGS, (char*)0);
	}
	else if(child_pid > 0){
		/* parent */
		return 0;
	}
	else{
		/* error */
		return -1;
	}
}

int gdbif::sigsend(int sig){
	sigval v;

	return sigqueue(child_pid, sig, v);
}

/**
 * \brief	read from gdb terminal
 *
 * \param	buf		target buffer
 * \param	nbytes	max bytes to read
 *
 * \return	number of read bytes on success
 * 			-1 on error
 */
int gdbif::read(void* buf, unsigned int nbytes){
	return child_term->read(buf, nbytes);
}

/**
 * \brief	write to gdb terminal
 *
 * \param	buf		source buffer
 * \param	nbytes	number of bytes to write
 *
 * \return	number of written bytes on success
 * 			-1 on error
 */
int gdbif::write(void* buf, unsigned int nbytes){
	return child_term->write(buf, nbytes);
}

/**
 * \brief	create gdb machine interface (MI) command
 *
 * \param	user_cmd			target command
 * \param	options		options to user_cmd
 * \param	noption		number of entries in options
 * \param	parameter	parameters to user_cmd
 * \param	nparameter	number of entries in parameter
 * \param	resp_hdlr	function to call upon gdb response
 *
 * \return	>0			token used for the command
 * 			-1			error
 */
response_t* gdbif::mi_issue_cmd(char* user_cmd, arglist_t* options, arglist_t* parameter){
	static char* cmd_str = 0;
	static unsigned int cmd_str_len = 0;
	unsigned int i, len;
	arglist_t* el;


	/* compute length of cmd_str */
	len = strlen(user_cmd) + strlen(token, 10) + 5;	// +5 = "-" " --" \0

	list_for_each(options, el){
		if(el->type == T_STRING)	len += strlen(el->value.sptr);
		else if(el->type == T_INT)	len += strlen(el->value.i, 10);

		len += 1 + (el->quoted ? 2 : 0);	// +1 = " "
	}

	list_for_each(parameter, el){
		if(el->type == T_STRING)	len += strlen(el->value.sptr);
		else if(el->type == T_INT)	len += strlen(el->value.i, 10);

		len += 1 + (el->quoted ? 2 : 0);	// +1 = " "
	}

	if(len > cmd_str_len){
		delete cmd_str;
		cmd_str = new char[len];

		if(cmd_str == 0){
			cmd_str_len = 0;
			return -1;
		}

		cmd_str_len = len;
	}

	/* assemble cmd_str */
	len = sprintf(cmd_str, "%d-%s", token, user_cmd);

	list_for_each(options, el){
		if(el->type == T_STRING)	len += sprintf(cmd_str + len, " %s%s%s", (el->quoted ? "\"" : ""), el->value, (el->quoted ? "\"" : ""));
		else if(el->type == T_INT)	len += sprintf(cmd_str + len, " %s%d%s", (el->quoted ? "\"" : ""), el->value, (el->quoted ? "\"" : ""));
	}

	if(options != 0)
		len += sprintf(cmd_str + len, " --");

	list_for_each(parameter, el){
		if(el->type == T_STRING)	len += sprintf(cmd_str + len, " %s%s%s", (el->quoted ? "\"" : ""), el->value, (el->quoted ? "\"" : ""));
		else if(el->type == T_INT)	len += sprintf(cmd_str + len, " %s%d%s", (el->quoted ? "\"" : ""), el->value, (el->quoted ? "\"" : ""));
	}

	/* issue user_cmd */
	if(this->write(cmd_str, len) < 0 || this->write((void*)"\n", 1) < 0)
		return 0;

	/* wait for gdb response */
	memset((void*)&resp, 0x0, sizeof(response_t));

	pthread_cond_wait(&resp_avail, &resp_mtx);

	if(resp_token != token){
		DEBUG("result token doesn't match issued token\n");
		return 0;
	}

	/* increment token */
	token++;

	return (response_t*)&resp;
}

int gdbif::mi_parse(char* s){
	gdb_scan_string(s);

	return (gdbparse(this) == 0 ? 0 : -1);
}

int gdbif::mi_proc_result(result_class_t rclass, unsigned int token, result_t* result){
	resp_token = token;
	resp.result = result;
	resp.rclass = rclass;

	pthread_cond_signal(&resp_avail);

	return 0;
}

int gdbif::mi_proc_async(result_class_t rclass, unsigned int token, result_t* result){
	/* TODO implement */
	TODO("not yet implemented\n");
	gdb_result_free(result);
	return 0;
}

int gdbif::mi_proc_stream(stream_class_t sclass, char* stream){
	/* TODO implement proper integration with log system */
	TODO("implement proper integration with log system\n");

	switch(sclass){
	case SC_CONSOLE:
		TEST("console stream: \"%s\"\n", stream);
		break;

	case SC_TARGET:
		TEST("target system stream: \"%s\"\n", stream);
		break;

	case SC_LOG:
		TEST("log stream: \"%s\"\n", stream);
		break;
	};

	return 0;
}

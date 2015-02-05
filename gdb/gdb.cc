#include <common/log.h>
#include <common/pty.h>
#include <gdb/gdb.h>
#include <gdb/lexer.lex.h>
#include <gdb/parser.tab.h>
#include <cmd/cmd.hash.h>
#include <cmd/subcmd.hash.h>
#include <string.h>
#include <math.h>


/* class definition */
/**
 * \brief	standard constructor
 */
gdb_if::gdb_if(){
	this->child_term = 0;
	this->token = 1;

	pthread_mutex_init(&resp_mutex, 0);
}

/**
 * \brief	standard desctructor
 */
gdb_if::~gdb_if(){
	// close gdb terminal
	delete this->child_term;
	this->child_term = 0;
}

/**
 * \brief	exec gdb and initialise its controlling terminal
 *
 * \return	0	on success
 * 			-1	on error (check errno)
 */
int gdb_if::init(){
	int pid;


	// initialise pseudo terminal
	this->child_term = new pty();

	// fork child process
	pid = child_term->fork();

	if(pid == 0){
		/* child */
		log::cleanup();

		return execl(GDB_CMD, GDB_CMD, GDB_ARGS, (char*)0);
	}
	else if(pid > 0){
		/* parent */
		return 0;
	}
	else{
		/* error */
		return -1;
	}
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
int gdb_if::read(void* buf, unsigned int nbytes){
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
int gdb_if::write(void* buf, unsigned int nbytes){
	return child_term->write(buf, nbytes);
}

/**
 * \brief	thread-safe enqueue, dequeue functions for issues gdb commands
 *
 * \param	token	token used for gdb MI command
 * \param	hdlr	response handler to associate with token
 *
 * \return	0		success
 * 			-1		error
 */
int gdb_if::resp_enqueue(unsigned int token, response_hdlr_t hdlr){
	pthread_mutex_lock(&resp_mutex);

	if(resp_map.find(token) == resp_map.end())
		return -1;

	resp_map[token] = hdlr;

	pthread_mutex_unlock(&resp_mutex);

	return 0;
}

int gdb_if::resp_dequeue(unsigned int token){
	map<unsigned int, response_hdlr_t>::iterator it;


	pthread_mutex_lock(&resp_mutex);

	it = resp_map.find(token);
	if(it == resp_map.end())
		return -1;

	resp_map.erase(it);

	pthread_mutex_unlock(&resp_mutex);

	return 0;
}

/**
 * \brief	create gdb machine interface (MI) command
 *
 * \param	cmd			target command
 * \param	options		options to cmd
 * \param	noption		number of entries in options
 * \param	parameter	parameters to cmd
 * \param	nparameter	number of entries in parameter
 * \param	resp_hdlr	function to call upon gdb response
 *
 * \return	>0			token used for the command
 * 			-1			error
 */
int gdb_if::mi_issue_cmd(char* cmd, char** options, unsigned int noption, char** parameter, unsigned int nparameter, response_hdlr_t resp_hdlr){
	static char* cmd_str = 0;
	static unsigned int cmd_str_len = 0;
	static unsigned int token_len = 1;
	unsigned int i, len;


	DEBUG("cmd: \"%s\"\n", cmd);

	/* compute length of cmd_str */
	len = strlen(cmd) + token_len + 5;	// +5 = "-" " --" "\n"

	for(i=0; i<noption; i++)
		len += strlen(options[i]) + 1;	// +1 = " "

	for(i=0; i<nparameter; i++)
		len += strlen(parameter[i]) + 1;	// +1 = " "

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
	len = sprintf(cmd_str, "%d-%s", token, cmd);

	for(i=0; i<noption; i++)
		len += sprintf(cmd_str + len, " %s", options[i]);

	if(noption > 0)
		len += sprintf(cmd_str + len, " --");

	for(i=0; i<nparameter; i++)
		len += sprintf(cmd_str + len, " %s", parameter[i]);

	len += sprintf(cmd_str + len, "\n");


	/* enqueue response handler */
	resp_enqueue(token, resp_hdlr);

	/* increment token */
	if(++token >= (unsigned int)pow(10, token_len))
		token_len++;

	/* issue cmd */
	if(this->write(cmd_str, len) < 0){
		resp_dequeue(token - 1);
		return -1;
	}

	return token - 1;
}

int gdb_if::mi_parse(char* s){
	DEBUG("parse gdb string \"%s\"\n", s);

	gdb_scan_string(s);

	return (gdbparse(this) == 0 ? 0 : -1);
}

int gdb_if::mi_proc_result(result_class_t rclass, unsigned int token, result_t* result){
	DEBUG("parsed result-record\n");
	return 0;
}

int gdb_if::mi_proc_async(async_class_t aclass, unsigned int token, result_t* result){
	DEBUG("parsed async-record\n");
	return 0;
}

int gdb_if::mi_proc_stream(stream_class_t sclass, char* stream){
	/* TODO implement proper integration with log system */

	switch(sclass){
	case SC_CONSOLE:
		DEBUG("console stream: \"%s\"\n", stream);
		break;

	case SC_TARGET:
		DEBUG("target system stream: \"%s\"\n", stream);
		break;

	case SC_LOG:
		DEBUG("log stream: \"%s\"\n", stream);
		break;
	};

	return 0;
}

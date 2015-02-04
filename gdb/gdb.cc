#include <common/log.h>
#include <common/pty.h>
#include <gdb/gdb.h>
#include <gdb/user_cmd.hash.h>
#include <gdb/user_subcmd.hash.h>
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

		return libc::execl(GDB_CMD, GDB_CMD, GDB_ARGS, (char*)0);
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
 * \brief	execute user command
 *
 * \param	cmdline		user command and arguments
 *
 * \return	0			sucess
 * 			-1			error
 */
int gdb_if::exec_user_cmd(char* cmdline){
	unsigned int nspace, i, len;
	int r;
	char** argv;


	len = strlen(cmdline);

	/* count number of spaces within cmdline */
	i = 0;
	nspace = 0;
	while(i < len){
		if(cmdline[i] == ' ')	nspace++;
		if(cmdline[i] == '"')	while(++i < len && cmdline[i] != '"');

		i++;
	}

	/* assign argv to space-separated strings in cmdline */
	argv = new char*[nspace + 1];
	argv[0] = cmdline;
	i = 0;
	nspace = 0;
	while(i < len){
		if(cmdline[i] == ' '){
			nspace++;
			argv[nspace] = cmdline + i + 1;
			cmdline[i] = 0;
		}

		if(cmdline[i] == '"'){
			argv[nspace]++;
			while(++i < len && cmdline[i] != '"');
			cmdline[i] = 0;
		}

		i++;
	}

	/* execute command */
	if(gdb_user_cmd::lookup(argv[0], strlen(argv[0])) <= 0){
		WARN("invalid user command \"%s\", trying direct execution by gdb\n", argv[0]);

		/* send unknown command to gdb */
		// reset all '\0' to ' '
		for(i=0; i<len; i++){
			if(cmdline[i] == 0) cmdline[i] = ' ';
		}

		// send cmdline to gdb
		this->write(cmdline, len);
		this->write((void*)"\n", 1);

		r = -1;
	}
	else
		r = gdb_user_cmd::lookup(argv[0], strlen(argv[0]))->callback(this, nspace + 1, argv);

	delete [] argv;
	return r;
}

int gdb_if::resp_enqueue(unsigned int token, response_hdlr hdlr){
	pthread_mutex_lock(&resp_mutex);

	if(resp_map.find(token) == resp_map.end())
		return -1;

	resp_map[token] = hdlr;

	pthread_mutex_unlock(&resp_mutex);

	return 0;
}

int gdb_if::resp_dequeue(unsigned int token){
	map<unsigned int, response_hdlr>::iterator it;


	pthread_mutex_lock(&resp_mutex);

	it = resp_map.find(token);
	if(it == resp_map.end())
		return -1;

	resp_map.erase(it);

	pthread_mutex_unlock(&resp_mutex);

	return 0;
}

int gdb_if::cmd_file(gdb_if* gdb, int argc, char** argv){
	unsigned int i;
	const char* cmd_str;
	const gdb_user_subcmd_t* subcmd;


	if(argc < 2){
		WARN("too few arguments to command \"%s\"\n", argv[0]);
		return -1;
	}

	if(argc > 3){
		WARN("too many arguments to command \"%s\", at most 2 expected\n", argv[0]);
		return -1;
	}

	cmd_str = "file-exec-and-symbols";

	for(i=1; i<argc-1; i++){
		subcmd = gdb_user_subcmd::lookup(argv[i], strlen(argv[i]));
		if(subcmd == 0){
			WARN("invalid sub command \"%s\" to \"%s\"\n", argv[i], argv[0]);
			return -1;
		}

		switch(subcmd->id){
		case BIN:
			cmd_str = "file-exec-file";
			break;

		case SYM:
			cmd_str = "file-symbol-file";
			break;
		};
	}

	// TODO add response handler
	if(gdb->mi_cmd_issue((char*)cmd_str, 0, 0, argv + i, 1, 0) < 0)
		return -1;
	return 0;
}

/**
 * \brief	user commands
 *
 * \param	gdb		pointer to gdb_if instance
 * \param	argc	number of entries in argv
 * \param	argv	arguments
 *
 * \return	0		success
 * 			<0		error
 */

/**
 * \brief	testing command
 */
int gdb_if::cmd_test(gdb_if* gdb, int argc, char** argv){
	unsigned int i;


	DEBUG("command: %s\n", argv[0]);
	DEBUG("arguments:");

	for(i=1; i<argc; i++)
		DEBUG(" (%d, \"%s\")", i, argv[i]);
	DEBUG("\n");
}

/**
 * \brief	help message
 */
int gdb_if::cmd_help(gdb_if* gdb, int argc, char** argv){
	unsigned int i;


	DEBUG("user commands:\n");

	for(i=gdb_user_cmd::MIN_HASH_VALUE; i<=gdb_user_cmd::MAX_HASH_VALUE; i++)
		DEBUG("    %7.7s   %s\n", gdb_user_cmd::wordlist[i].name, gdb_user_cmd::wordlist[i].help_msg);
	return 0;
}

int gdb_if::mi_cmd_issue(char* cmd, char** options, unsigned int noption, char** parameter, unsigned int nparameter, response_hdlr resp_hdlr){
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

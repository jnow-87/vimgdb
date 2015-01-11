#include <common/log.h>
#include <common/pty.h>
#include <gdb/gdb.h>
#include <gdb/user_cmd.hash.h>
#include <string.h>


/* class definition */
/**
 * \brief	standard constructor
 */
gdb_if::gdb_if(){
	this->child_term = 0;
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


	printf("command: %s\n", argv[0]);
	printf("arguments:");

	for(i=1; i<argc; i++)
		printf(" (%d, \"%s\")", i, argv[i]);
	printf("\n");
}

/**
 * \brief	help message
 */
int gdb_if::cmd_help(gdb_if* gdb, int argc, char** argv){
	unsigned int i;


	printf("user commands:\n");

	for(i=MIN_HASH_VALUE; i<=MAX_HASH_VALUE; i++)
		printf("    %7.7s   %s\n", wordlist[i].name, wordlist[i].help_msg);
	return 0;
}

#include <common/log.h>
#include <common/pty.h>
#include <common/string.h>
#include <gdb/gdb.h>
#include <user_cmd/cmd.hash.h>
#include <user_cmd/subcmd.hash.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <unistd.h>


/* class definition */
/**
 * \brief	standard constructor
 */
gdbif::gdbif(){
	gdb = 0;
	token = 1;

	pthread_mutex_init(&resp_mtx, 0);
	pthread_cond_init(&resp_avail, 0);
}

/**
 * \brief	standard desctructor
 */
gdbif::~gdbif(){
	// close gdb terminal
	delete this->gdb;
	this->gdb = 0;

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
	this->gdb = new pty();

	// fork child process
	pid = gdb->fork();

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

int gdbif::sigsend(int sig){
	sigval v;

	return sigqueue(pid, sig, v);
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
	return gdb->read(buf, nbytes);
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
	return gdb->write(buf, nbytes);
}

/**
 * \brief	create gdb machine interface (MI) command
 *
 * \param	user_cmd	target command
 * \param	fmt			printf-like format string, describing the following parameters
 * 						supported identifiers:
 * 							'%d'		integer
 * 							'%s'		string
 * 							'%ss %d'	array of strings, length is defined by the
 * 										following integer
 * 							'--'		separator between options and parameters
 * 							.			everything else except blanks is printed literaly
 *
 * \param	...			parameters according to param_fmt
 *
 * \return	>0			token used for the command
 * 			-1			error
 */
gdb_response_t* gdbif::mi_issue_cmd(char* user_cmd, const char* fmt, ...){
	static char* s = 0;
	static unsigned int s_len = 0;
	unsigned int i, j, argc;
	char** argv;
	va_list lst;


	va_start(lst, fmt);

	gdb->write(itoa(token, &s, &s_len));
	gdb->write((char*)"-");
	gdb->write(user_cmd);

	for(i=0; i<strlen(fmt); i++){
		gdb->write((char*)" ");

		switch(fmt[i]){
		case '%':
			switch(fmt[i + 1]){
			case 'd':
				gdb->write(itoa(va_arg(lst, int), &s, &s_len));
				i++;
				break;

			case 's':
				if(strncmp(fmt + i + 2, "s %d", 4) == 0){
					argv = va_arg(lst, char**);
					argc = va_arg(lst, int);

					for(j=0; j<argc; j++)
						gdb->write(argv[j]);

					i += 4;
				}
				else
					gdb->write(va_arg(lst, char*));

				i++;
				break;

			default:
				ERROR("invalid format sequence %%%c\n", fmt[i + 1]);
				return 0;
			};

			break;

		default:
			gdb->write((void*)(fmt + i), 1);
			break;
		};
	}

	/* wait for gdb response */
	pthread_mutex_lock(&resp_mtx);

	memset((void*)&resp, 0x0, sizeof(gdb_response_t));

	gdb->write((char*)"\n");	// ensure that response cannot arrive
								// before it is expected

	pthread_cond_wait(&resp_avail, &resp_mtx);

	token++;

	pthread_mutex_unlock(&resp_mtx);

	return (gdb_response_t*)&resp;
}

int gdbif::mi_proc_result(gdb_result_class_t rclass, unsigned int token, gdb_result_t* result){
	pthread_mutex_lock(&resp_mtx);

	if(this->token != token)
		ERROR("result token (%d) doesn't match issued token (%d)\n", token, this->token);

	resp.result = result;
	resp.rclass = rclass;

	pthread_cond_signal(&resp_avail);
	pthread_mutex_unlock(&resp_mtx);

	return 0;
}

int gdbif::mi_proc_async(gdb_result_class_t rclass, unsigned int token, gdb_result_t* result){
	/* TODO implement */
	TODO("not yet implemented\n");
	gdb_result_free(result);
	return 0;
}

int gdbif::mi_proc_stream(gdb_stream_class_t sclass, char* stream){
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

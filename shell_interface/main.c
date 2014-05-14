#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include "pty.h"


typedef struct{
	int fd_in, fd_out;
	char token[15];
} arg_t;


void* read_fd(void* arg);
void sig_hdlr(int signum);


char name[255];
int pid, fd_master;


/**
 * \brief	working example of how to create child process and
 * 			associated terminal for communication
 *
 * 			relies on ./forkee as exec'ed process
 * 			sig_hdlr ensuring correct shutdown, once forkee exits
 */
int main(int argc, char** argv){
	pthread_t tid;
	arg_t a;


	if((pid = forkpty(&fd_master, 0, 0)) == 0){
		return execl("./forkee", "forkee", "", 0);
	}
	else if(pid > 0){
		signal(SIGCHLD, sig_hdlr);
		sprintf(name, "parent");
		printf("parent: child_pid %d, child_fd %d\n", pid, fd_master);

		a.fd_in = fd_master;
		a.fd_out = 1;
		strcpy(a.token, "gdb: ");

		pthread_create(&tid, 0, read_fd, (void*)&a);

		// wait for thread to be created, since otherwise
		// a.fd_in, a.fd_out are overwritten
		sleep(1);

		a.fd_in = 0;
		a.fd_out = fd_master;
		strcpy(a.token, "");

		read_fd((void*)&a);
	}
	else{
		printf("error\n");
		return 1;
	}

	return 0;
}


void* read_fd(void* arg){
	char c, line[1024], o[1024];
	unsigned int n;
	int r;
	arg_t a;


	a = *((arg_t*)arg);
	n = 0;

	while(read(a.fd_in, &c, 1) == 1){
		if(c == '\n' || c == '\r'){
			line[n] = 0;

			if(n > 0){
				sprintf(o, "%s%s\n", a.token, line);
				write(a.fd_out, o, strlen(o));
				n = 0;
			}
		}
		else{
			line[n++] = c;
		}
	}

	return 0;
}

void sig_hdlr(int signum){
	printf("child exited\n");

	waitpid(pid, 0, 0);
	close(fd_master);
}

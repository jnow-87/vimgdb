#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>


typedef struct{
	int fd_in, fd_out;
} arg_t;

char str[255];

void* read_fd(void* arg);


int main(int argc, char** argv){
	unsigned int i;
	pthread_t tid;
	arg_t arg;


	printf("forkee should be called by the shell_interface binary\n");
	for(i=0; i<argc; i++)
		printf("%d: %s\n", i, argv[i]);

	arg.fd_in = 0;
	arg.fd_out = 1;
	pthread_create(&tid, 0, read_fd, &arg);

	strcpy(str, "forkee");

	while(1){
		if(strcmp(str, "quit") == 0)
			return 0;

		printf("%s\n", str);
		sleep(1);
	}

	return 0;
}


void* read_fd(void* arg){
	char c, line[1024], o[1024];
	unsigned int n;
	arg_t a;


	a = *((arg_t*)arg);
	n = 0;

	while(1){
		read(a.fd_in, &c, 1);

		if(c == '\n' || c == '\r'){
			line[n] = 0;
			strcpy(str, line);
			n = 0;

			if(strcmp(str, "quit") == 0)
				return 0;
		}
		else
			line[n++] = c;
	}
}

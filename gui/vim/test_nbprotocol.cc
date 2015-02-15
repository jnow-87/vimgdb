#include <common/socket.h>
#include <common/log.h>
#include <common/string.h>
#include <cmd.hash.h>
#include <fct.hash.h>
#include <event.h>
#include <lexer.lex.h>
#include <parser.tab.h>
#include <event.hash.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


/* types */
typedef enum{
	FCT = 1,
	CMD,
} vim_action_t;

/* static prototypes */
static void* thread_server(void* arg);
static int vim_action(vim_action_t action, int argc, char** argv);


/* static variables */
static socket* client;


int main(int argc, char** argv){
	char line[255], c;
	unsigned int i;
	int cmd_argc;
	char** cmd_argv;
	socket *server;
	pthread_t tid;
	const vim_cmd_t* cmd;
	const vim_fct_t* fct;


	log::init("/dev/stdout", (log_level_t)(INFO | WARN | ERROR | DEBUG | USER | TEST | TODO));


	client = 0;
	server = new socket(VIM_NB_PORT, 0);
	if(server->init_server(TCP) != 0){
		delete server;
		log::cleanup();
		return 1;
	}

	pthread_create(&tid, 0, thread_server, server);

	i = 0;
	while(1){
		if(read(0, &c, 1) == 1){
			if(c == '\r')
				continue;

			if(c == '\n'){
				line[i] = 0;

				if(strcmp(line, "q") == 0 || strcmp(line, "quit") == 0)
					break;

				for(i=0; i<strlen(line); i++){
					if(line[i] == ' ')
						break;
				}

				cmd = vim_cmd::lookup(line, i);
				fct = vim_fct::lookup(line, i);

				if(cmd != 0 || fct != 0){
					if(strsplit(line, &cmd_argc, &cmd_argv) != 0){
						USER("error splitting line\n");
						continue;
					}

					if(cmd != 0)	vim_action(CMD, cmd_argc, cmd_argv);
					else			vim_action(FCT, cmd_argc, cmd_argv);

					for(i=0; i<cmd_argc; i++)
						delete cmd_argv[i];
					delete cmd_argv;
				}
				else{
					if(client != 0){
						USER("direct execution \"%s\" %d\n", line, strlen(line));

						client->send(line, strlen(line));
						client->send((void*)"\n", 1);
					}
					else
						USER("no client\n");
				}

				i = 0;
			}
			else
				line[i++] = c;
		}
		else
			break;
	}


	pthread_cancel(tid);
	pthread_join(tid, 0);

	delete server;
	log::cleanup();

	return 0;
}


void* thread_server(void* arg){
	char c, *line;
	unsigned int i, len;
	socket *server;


	server = (socket*)arg;
	i = 0;
	len = 255;
	line = (char*)malloc(len * sizeof(char));

	if(line == 0)
		return 0;

	while(1){
		client = server->await_client();

		if(client != 0){
			USER("start new client thread for %s\n", client->get_ip());

			client->set_timeout(0);

			while(1){
				if(client->recv(&c, 1) > 0){
					if(c == '\r')
						continue;

					if(i >= len){
						len *= 2;
						line = (char*)realloc(line, len);

						if(line == 0)
							goto err;
					}

					if(c == '\n'){

						line[i] = 0;
						USER("parse: \"%s\"\n", line);

						line[i++] = '\n';
						line[i] = 0;
						vim_scan_string(line);
						USER("parser return value %d\n", vimparse());

						i = 0;
					}
					else
						line[i++] = c;
				}
				else{
					USER("detect client shutdown\n");
					break;
				}
			}

			delete client;
			client = 0;
		}
	}

err:
	delete client;
	pthread_exit(0);
}

int vim_action(vim_action_t action, int argc, char** argv){
	static unsigned int seq_num = 1;
	unsigned int i;
	char seq_str[strlen(seq_num, 10) + 1];


	if(client == 0){
		USER("no client\n");
		return -1;
	}

	sprintf(seq_str, "%d", seq_num);

	client->send(argv[1], strlen(argv[1]));	// buffer number
	client->send((void*)":", 1);
	client->send(argv[0], strlen(argv[0]));

	if(action == FCT)		client->send((void*)"/", 1);
	else if(action == CMD)	client->send((void*)"!", 1);
	client->send(seq_str, strlen(seq_str));

	for(i=2; i<argc; i++){
		client->send((void*)" ", 1);
		client->send(argv[i], strlen(argv[i]));
	}

	client->send((void*)"\n", 1);
	return 0;
}

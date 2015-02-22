#ifndef SOCKET_H
#define SOCKET_H


/* types */
enum socket_t{
	TCP = 1,
	UDP,
};


/* class */
class socket{
public:
	socket(int port, const char* addr);
	~socket();

	int init_client(socket_t type);
	int init_server(socket_t type);
	socket* await_client();

	int recv(void* data, int size);
	int send(void* data, int size);
	int send(char* s);

	int set_timeout(int sec);
	int set_bcast(int value);
	int set_ip(const char* ip);
	char* get_ip(char* ip = 0);
	int get_timeout();

private:
	// std constructor (private to prevent from calling)
	socket();

	int fd_sock, timeout;
	void* saddr;	// is of type sockaddr_in* but has to be void*
					// to avoid inclusion of arpa/inet.h to avoid
					// name collision with socket()
};


#endif	// SOCKET_H

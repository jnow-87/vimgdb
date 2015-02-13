#include <common/log.h>
#include <common/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

namespace libc{
	// cover in separate namespace to avoid
	// name collision with socket()
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <sys/socket.h>
}

using namespace libc;


socket::socket(){
	saddr = (sockaddr_in*)malloc(sizeof(sockaddr_in));
	memset(saddr, 0, sizeof(sockaddr_in));
	((sockaddr_in*)saddr)->sin_family = AF_INET;
}

/**
 * \brief	constructor
 *
 * \param	port	port-number
 * \param	addr	targets ip
 */
socket::socket(int port, const char* addr){
	fd_sock = -1;
	timeout = 0;

	saddr = (sockaddr_in*)malloc(sizeof(sockaddr_in));
	memset(saddr, 0, sizeof(sockaddr_in));
	((sockaddr_in*)saddr)->sin_family = AF_INET;
	((sockaddr_in*)saddr)->sin_port = htons(port);

	if(addr == 0)
		((sockaddr_in*)saddr)->sin_addr.s_addr = htonl(INADDR_ANY);
	else
		((sockaddr_in*)saddr)->sin_addr.s_addr = inet_addr(addr);
}

/**
 * \brief	destructor to free mem
 */
socket::~socket(){
	DEBUG("closing socket to %s\n", get_ip());

	free(saddr);

	if(fd_sock != -1){
		if(close(fd_sock) != 0)
			ERROR("closing socket failed with \"%s\"\n", strerror(errno));
	}
}

/**
 * \brief	intialize the socket to use as client
 *
 * \param	type	socket-type (TCP, UDP)
 * \return			0 on success
 * 					-1 on error
 */
int socket::init_client(socket_t type){
	int sock_type;


	sock_type = (type == TCP) ? SOCK_STREAM : SOCK_DGRAM;

	if((fd_sock = libc::socket(AF_INET, sock_type, 0)) == -1){
		ERROR("init_client failed with \"%s\"\n", strerror(errno));
		return -1;
	}

	if(sock_type == SOCK_DGRAM)
		return 0;

	if(connect(fd_sock, (sockaddr*)saddr, sizeof(sockaddr_in)) == -1){
		close(fd_sock);
		return -1;
	}

	return 0;
}

/**
 * \brief	intialize the socket to use as server
 *
 * \param	type	socket-type (TCP, UDP)
 * \return			0 on success
 * 					-1 on error
 */
int socket::init_server(socket_t type){
	int sock_type;


	sock_type = (type == TCP) ? SOCK_STREAM : SOCK_DGRAM;

	if((fd_sock = libc::socket(AF_INET, sock_type, 0)) == -1){
		ERROR("init_server failed with \"%s\"\n", strerror(errno));
		return -1;
	}

	if(bind(fd_sock, (sockaddr*)saddr, sizeof(sockaddr_in)) < 0 && errno != EINVAL){
		ERROR("bind server failed with \"%s\"\n", strerror(errno));
		close(fd_sock);
		return (fd_sock = -1);
	}

	if(sock_type == SOCK_DGRAM)
		return 0;

	if(listen(fd_sock, 4) != 0){
		close(fd_sock);
		return (fd_sock = -1);
	}

	return 0;
}

/**
 * \brief	accept-syscall
 *
 * \return		new socket to remote host
 */
class socket* socket::await_client(){
	int fd_client_sock;
	socklen_t size;
	sockaddr_in saddr_in;
	socket* client;


	size = sizeof(sockaddr_in);

	if((fd_client_sock = accept(fd_sock, (sockaddr*)&saddr_in, &size)) == -1){
		return 0;
	}

	client = new class socket();
	client->fd_sock = fd_client_sock;
	memcpy(client->saddr, &saddr_in, size);

	return client;
}

/**
 * \brief	receive function on raw data
 *
 * \param	data	any kind of data
 * \param	size	size of data-buffer
 * \return			number of bytes received, or -1 on error, 0 when peer has performed an orderly shutdown.
 */
int socket::recv(void* data, int size){
	socklen_t saddr_size;

	return recvfrom(fd_sock, data, size, 0, (sockaddr*)saddr, &saddr_size);
}

/**
 * \brief	send function on raw data
 *
 * \param	data	any kind of data
 * \param	size	size of data-buffer
 * 	\return			number of bytes send on succes,  -1 on error
 */
int socket::send(void* data, int size){
	return sendto(fd_sock, data, size, 0, (sockaddr*)saddr, sizeof(sockaddr_in));
}

/**
 * \brief	set timeout for all socket-related functions
 *
 * \param	sec		timout in seconds
 * \return			0 on success, -1 on error
 */
int socket::set_timeout(int sec){
	struct timeval tv;

	timeout = sec;
	tv.tv_sec = sec;
	tv.tv_usec = 0;

	return setsockopt(fd_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(timeval));
}

/**
 * \brief	set broadcast option for all socket-related functions
 *
 * \param	value	0 to disable, else enable
 * \return			0 on success, -1 on error
 */
int socket::set_bcast(int value){
	if(value == 0) ((sockaddr_in*)saddr)->sin_addr.s_addr = INADDR_ANY;
	else ((sockaddr_in*)saddr)->sin_addr.s_addr = INADDR_BROADCAST;

	return setsockopt(fd_sock, SOL_SOCKET, SO_BROADCAST, (const char *)&value, sizeof(int));
}

/**
 * \brief	set ip for target socket
 *
 * \param	ip		ip
 * \return			0 on success, -1 on error
 */
int socket::set_ip(const char* ip){
	if(ip == 0)
		return -1;

	((sockaddr_in*)saddr)->sin_addr.s_addr = inet_addr(ip);
	return 0;
}

/**
 * \brief	get ip from current partner-socket
 *
 * \paran	ip	pointer where to store ip
 * \return		if zero is given as argument, return a pointer to string containing the ip (statically alloced mem)
 * 				if non zero argument is given, return ip
 */
char* socket::get_ip(char* ip){
	if(ip != 0){
		strcpy(ip, inet_ntoa(((sockaddr_in*)saddr)->sin_addr));
		return ip;
	}

	return inet_ntoa(((sockaddr_in*)saddr)->sin_addr);
}

/**
 * \brief	get timeout for socket
 *
 * \return		timeout
 */
int socket::get_timeout(){
	return timeout;
}

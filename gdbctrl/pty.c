#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include "pty.h"


int forkpty(int* _fd_master, struct termios* termp, struct winsize* win_size){
	int fd_master, fd_slave, pid;


	// open pseudoterminal
	if(openpty(&fd_master, &fd_slave, termp, win_size) != 0)
		return -1;

	pid = fork();
	switch(pid){
	// error
	case -1:
		close(fd_master);
		close(fd_slave);

		return -1;

	// child
	case 0:
		close(fd_master);
		*_fd_master = -1;

		// make fd_slave controlling terminal
		if(loginpty(fd_slave) < 0)
			exit(1);

		return 0;

	// parent
	default:
		close(fd_slave);
		*_fd_master = fd_master;

		return pid;
	}
}

int openpty(int* _fd_master, int* _fd_slave, struct termios* termp, struct winsize* win_size){
	int fd_master, fd_slave;
	char name[255];


	// open new pseudo terminal (pts)
	fd_master = getpt();
	if(fd_master == -1)
		return -1;

	// change mode and owner of pts slave side according to fd_master
	if(grantpt(fd_master))
		goto err_0;

	// unlock access to slave side of fd_master
	if(unlockpt(fd_master))
		goto err_0;

	if(ptsname_r(fd_master, name, 255) != 0)
		goto err_0;

	// open terminal without making it the controlling terminal
	fd_slave = open(name, O_RDWR | O_NOCTTY);
	if(fd_slave == -1)
		goto err_0;

	// applying terminal parametr
	if(termp){
		if(tcsetattr(fd_slave, TCSAFLUSH, termp) != 0)
			goto err_1;
	}

	if(win_size){
		if(ioctl(fd_slave, TIOCSWINSZ, win_size) == -1)
			goto err_1;
	}

	*_fd_master = fd_master;
	*_fd_slave = fd_slave;

	return 0;

err_1:
	close(fd_slave);

err_0:
	close(fd_master);
	return -1;
}

int loginpty(int fd){
	// create new session
	setsid();

	// set controlling terminal
	if(ioctl(fd, TIOCSCTTY, 0) == -1)
		return -1;

	// create standard file descriptors
	if(dup2(fd, 0) == -1 || dup2(fd, 1) == -1 || dup2(fd, 2) == -1)
		return -1;

	if(fd > 2)
		close(fd);

	return 0;
}

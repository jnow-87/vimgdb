#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "log.h"
#include "pty.h"


pty::pty() : pty(0, 0){
}

pty::pty(struct termios* termp, struct winsize* win_size){
	// open pseudoterminal
	if(openpty(&fd_master, &fd_slave, termp, win_size) != 0){
		this->fd_master = 0;
		this->fd_slave = 0;
	}

	this->forkee_pid = -1;
}

pty::~pty(){
	// killing gdb child process
	if(this->forkee_pid != -1){
		// prevent SIGCLD from being triggered
		signal(SIGCHLD, 0);

		kill(this->forkee_pid, SIGTERM);
		waitpid(this->forkee_pid, 0, 0);
		this->forkee_pid = -1;
	}

	// close terminal
	close(this->fd_master);
	close(this->fd_slave);
}

int pty::forkpty(){
	this->forkee_pid = fork();

	switch(this->forkee_pid){
	// error
	case -1:
		close(this->fd_master);
		close(this->fd_slave);

		return -1;

	// child
	case 0:
		close(this->fd_master);
		this->fd_master = 0;

		// make fd_slave controlling terminal
		if(login(this->fd_slave) < 0)
			exit(1);

		return 0;

	// parent
	default:
		close(this->fd_slave);

		// register signal handler for SIGCHLD
		if(signal(SIGCHLD, pty::sig_hdlr_chld) == SIG_ERR){
			ERROR("register sig_hdlr_chld() failed\n");
			return -1;
		}


		return forkee_pid;
	}
}

int pty::read(char* buf, unsigned int max_size){
	// TODO
	return 0;
}

int pty::write(char* but, unsigned int size){
	// TODO
	return 0;
}

int pty::openpty(int* _fd_master, int* _fd_slave, struct termios* termp, struct winsize* win_size){
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

int pty::login(int fd){
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

void pty::sig_hdlr_chld(int signum){
	INFO("caught child signal %d, initialising cleanup\n", signum);

	// kill self
	kill(getpid(), SIGTERM);
}

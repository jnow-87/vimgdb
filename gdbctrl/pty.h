#ifndef PTY_H
#define PTY_H


#include "tty.h"


/* class */
class pty : public tty{
public:
	/* constructor/desctructor */
	pty();
	pty(struct termios* termp, struct winsize* win_size);
	~pty();

	/* fork process */
	int fork();

private:
	int fd_master,		// file descriptor for master side of PTY
		fd_slave,		// file descriptor for slave side of PTY
		forkee_pid;		// PID in case fork has been used

	/* open new pseudo terminal */
	static int openpty(int* fd_maser, int* fd_slave, struct termios* termp, struct winsize* win_size);

	/* make given file descriptor the controlling terminal */
	static int login(int fd);

	/* signal handler */
	static void sig_hdlr_chld(int signum);
};


#endif

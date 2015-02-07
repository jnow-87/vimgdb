#ifndef PTY_H
#define PTY_H


#include <common/tty.h>


/* class */
class pty : public tty{
public:
	/* constructor/desctructor */
	pty();
	pty(struct termios* termp, struct winsize* win_size);
	~pty();

	/* fork process */
	int fork();

	/* getter */
	char* get_name();

private:
	int fd_master,		// file descriptor for master side of PTY
		fd_slave,		// file descriptor for slave side of PTY
		forkee_pid;		// PID in case fork has been used

	char name[255];	// path to pty in filesystem (/dev)

	/* open new pseudo terminal */
	int openpty(int* fd_maser, int* fd_slave, struct termios* termp, struct winsize* win_size);

	/* make given file descriptor the controlling terminal */
	int login(int fd);

	/* signal handler */
	static void sig_hdlr_chld(int signum);
};


#endif

#include <common/log.h>
#include <common/pty.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <signal.h>

namespace libc{
	// cover in separate namespace to avoid name collision
	#include <unistd.h>
}


/* class definition */
/**
 * \brief	standard Constructor
 */
pty::pty() : pty(0, 0){
}

/**
 * \brief	constructor - open pseudo terminal
 *
 * \param	termp		terminal parameter applied to the slave side
 * \param	win_size	window parameter applied to the slave side
 */
pty::pty(struct termios* termp, struct winsize* win_size){
	// open pseudoterminal
	if(openpty(&fd_master, &fd_slave, termp, win_size) != 0){
		fd_master = 0;
		fd_slave = 0;
	}

	fd_in = fd_master;
	fd_out = fd_master;
	forkee_pid = -1;
}

/**
 * \brief	standard destructor
 */
pty::~pty(){
	// killing gdb child process
	if(forkee_pid != -1){
		kill(forkee_pid, SIGTERM);
		waitpid(forkee_pid, 0, 0);
		forkee_pid = -1;
	}

	// close terminal
	libc::close(fd_master);
	libc::close(fd_slave);
}

/**
 * \brief	fork process, using pseudo terminal as interface
 *
 * \param	fd_master	on success this holds the file descriptor to the master side
 * \param	termp		terminal parameter applied to the slave side
 * \param	win_size	window parameter applied to the slave side
 *
 * \return	0			on child
 * 			>0			on parent
 * 			-1			on error
 */
int pty::fork(){
	forkee_pid = libc::fork();

	switch(forkee_pid){
	// error
	case -1:
		libc::close(fd_master);
		libc::close(fd_slave);

		return -1;

	// child
	case 0:
		libc::close(fd_master);
		fd_master = 0;

		// make fd_slave controlling terminal
		if(login(fd_slave) < 0)
			exit(1);

		return 0;

	// parent
	default:
		libc::close(fd_slave);
		return forkee_pid;
	}
}


char* pty::get_name(){
	return name;
}

/**
 * \brief	open new pts, making it the controlling terminal
 *
 * \param	fd_master	on success this holds the file descriptor to the master side
 * \param	fd_slave	on success this holds the file descriptor to the slave side
 * \param	termp		terminal parameter applied to the slave side
 * \param	win_size	window parameter applied to the slave side
 *
 * \return	0			on success
 * 			-1			on error
 */
int pty::openpty(int* _fd_master, int* _fd_slave, struct termios* termp, struct winsize* win_size){
	int fd_master, fd_slave;


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
	libc::close(fd_slave);

err_0:
	libc::close(fd_master);
	return -1;
}

/**
 * \brief	make given file descriptor the controlling terminal
 *
 * \param	fd			file descriptor
 *
 * \return	0			on success
 * 			-1			on error
 */
int pty::login(int fd){
	// create new session
	libc::setsid();

	// set controlling terminal
	if(ioctl(fd, TIOCSCTTY, 0) == -1)
		return -1;

	// create standard file descriptors
	if(libc::dup2(fd, 0) == -1 || libc::dup2(fd, 1) == -1 || libc::dup2(fd, 2) == -1)
		return -1;

	if(fd > 2)
		libc::close(fd);

	return 0;
}

#ifndef PTY_H
#define PTY_H


#include <termios.h>


class pty{
public:
	pty();

	/**
	 * \brief	open pseudo terminal
	 *
	 * \param	termp		terminal parameter applied to the slave side
	 * \param	win_size	window parameter applied to the slave side
	 */
	pty(struct termios* termp, struct winsize* win_size);
	~pty();


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
	int forkpty();
	int read(char* buf, unsigned int max_size);
	int write(char* but, unsigned int size);

private:
	int fd_master, fd_slave, forkee_pid;

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
	static int openpty(int* fd_maser, int* fd_slave, struct termios* termp, struct winsize* win_size);

	/**
	 * \brief	make given file descriptor the controlling terminal
	 *
	 * \param	fd			file descriptor
	 *
	 * \return	0			on success
	 * 			-1			on error
	 */
	static int login(int fd);

	/**
	 * \brief	signal handler
	 *
	 * \param	signal number
	 */
	static void sig_hdlr_chld(int signum);
};


#endif

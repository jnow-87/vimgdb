#ifndef PTY_H
#define PTY_H


#include <asm-generic/termios.h>


/**
 * \brief	fork process, creating related controlling terminal
 *
 * \param	fd_master	on success this holds the file descriptor to the master side
 * \param	termp		terminal parameter applied to the slave side
 * \param	win_size	window parameter applied to the slave side
 *
 * \return	0			on child
 * 			>0			on parent
 * 			-1			on error
 */
int forkpty(int* fd_master, struct termios* termp, struct winsize* win_size);

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
int openpty(int *fd_master, int* fd_slave, struct termios* termp, struct winsize* win_size);

/**
 * \brief	make given file descriptor the controlling terminal
 *
 * \param	fd			file descriptor
 *
 * \return	0			on success
 * 			-1			on error
 */
int loginpty(int fd);


#endif

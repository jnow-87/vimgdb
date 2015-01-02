#include <common/tty.h>

namespace libc{
	// cover in separate namespace to avoid name collision
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <unistd.h>
}


/* class definition */
/**
 * \brief	standard constructor
 */
tty::tty(){
	this->fd_in = 0;
	this->fd_out = 1;
}

/**
 * \brief	extended constructor
 *
 * \param	in_file		file to read from
 * \param	out_file	file to write to
 */
tty::tty(const char* in_file, const char* out_file){
	this->fd_in = libc::open(in_file, O_RDONLY);
	this->fd_out = libc::open(out_file, O_WRONLY);
}

/**
 * \brief	destructor
 */
tty::~tty(){
	if(this->fd_in > 1)
		libc::close(this->fd_in);

	if(this->fd_out > 1)
		libc::close(this->fd_out);
}

/**
 * \brief	read from fd_in
 *
 * \param	buf		target buffer
 * \param	nbytes	max bytes to read
 *
 * \return	number of read bytes on success
 * 			-1 on error
 */
int tty::read(void* buf, unsigned int nbytes){
	// TODO
	return libc::read(fd_in, buf, nbytes);
}

/**
 * \brief	write to fd_out
 *
 * \param	buf		source buffer
 * \param	nbytes	number of bytes to write
 *
 * \return	number of written bytes on success
 * 			-1 on error
 */
int tty::write(void* buf, unsigned int nbytes){
	// TODO
	return libc::write(fd_out, buf, nbytes);
}

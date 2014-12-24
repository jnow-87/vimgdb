#ifndef TTY_H
#define TTY_H


/* class */
class tty{
public:
	/* constructor/destructor */
	tty();
	tty(const char* in_file, const char* out_file);
	~tty();

	/* read/write from/to file descriptor fd_in/fd_out */
	int read(void* buf, unsigned int nbytes);
	int write(void* buf, unsigned int nbytes);

protected:
	int fd_in,		// file descriptor used for reading
		fd_out;		// file descriptor used for writing
};


#endif

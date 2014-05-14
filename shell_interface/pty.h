#ifndef PTY_H
#define PTY_H


#include <asm-generic/termios.h>


int forkpty(int* fd_master, struct termios* termp, struct winsize* win_size);
int openpty(int *fd_master, int* fd_slave, struct termios* termp, struct winsize* win_size);
int openpty(int* _fd_master, int* _fd_slave, struct termios* termp, struct winsize* win_size);


#endif

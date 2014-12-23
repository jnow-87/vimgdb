#ifndef XALLOC_H
#define XMALLOC_H


void* xmalloc(unsigned int nbytes);
void xfree(void* p);

void xmalloc_init();
void xmalloc_eval();


#endif // XMALLOC_H

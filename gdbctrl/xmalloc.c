#include <stdlib.h>
#include <stdio.h>
#include <string.h>


static void* pmalloc[100];
static unsigned int idx = 0;


void* xmalloc(unsigned int nbytes){
	void* p;


	p = malloc(nbytes);
	printf("malloc %#x (%d bytes)\n", p, nbytes);
	pmalloc[idx++] = p;
	return p;
}

void xfree(void* p){
	unsigned int i, found;


	printf("free %#x\n", p);

	for(found=0, i=0; i<100; i++){
		if(pmalloc[i] == p){
			found = 1;
			pmalloc[i] = 0;
			break;
		}
	}

	if(!found)
		printf("%#x no longer allocated\n", p);

	free(p);
}


void xmalloc_init(){
	memset(pmalloc, 0x0, sizeof(void*) * 100);
}

void xmalloc_eval(){
	unsigned int i;


	for(i=0; i<100; i++){
		if(pmalloc[i] != 0)	printf("\033[31m%#x still alloced\033[0m\n", pmalloc[i]);
	}
}

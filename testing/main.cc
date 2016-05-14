#include <stdio.h>


int glob = 10;


int foo(char *s){
	s[0] = 'c';

	printf("%s\n", s);

	return 1;
}


int main(int argc, char **argv){
	char s[5];
	char *sx;


	sprintf(s, "halo");
	
	sx = new char[10];
	sprintf(sx, "testor");

	foo(s);

	printf("%s\n", s);
	printf("%s\n", sx);

	return 0;
}

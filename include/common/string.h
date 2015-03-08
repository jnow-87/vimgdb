#ifndef STRING_H
#define STRING_H


int strlen(int val, int base);
int strsplit(char* line, int* argc, char*** argv);
char* strescape(char* s, char** e, unsigned int* e_max);
char* itoa(int v, char** s, unsigned int* max);
char* stralloc(char* s, unsigned int len);



#endif // STRING_H

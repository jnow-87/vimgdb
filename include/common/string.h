#ifndef STRING_H
#define STRING_H


int strlen(int val, int base);
int strsplit(char* line, int* argc, char*** argv);
char* strescape(char* s);
char* itoa(int v);


#endif // STRING_H

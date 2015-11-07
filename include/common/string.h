#ifndef STRING_H
#define STRING_H


int strlen(unsigned int val, int base);
int strlen(unsigned long int val, int base);
int strlen(int val, int base);
int strlen(long int val, int base);
int strsplit(char* line, int* argc, char*** argv);
char* strescape(char* s, char** e, unsigned int* e_max);
char* strdeescape(char* s);
char* itoa(unsigned int v, char** s, unsigned int* max, unsigned int base, bool neg = false);
char* itoa(int v, char** s, unsigned int* max, unsigned int base);
char* itoa(long int v, char** s, unsigned int* max, unsigned int base);
char* itoa(unsigned long int v, char** s, unsigned int* max, unsigned int base, bool neg = false);
char* stralloc(char* s, unsigned int len);
void strswap2(char* s, unsigned int len);


#endif // STRING_H

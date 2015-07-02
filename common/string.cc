#include <common/log.h>
#include <common/string.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


int strlen(unsigned int val, int base){
	return strlen((unsigned long int)val, base);
}

int strlen(unsigned long int val, int base){
	unsigned int len;


	len = 1;
	while(1){
		if(val < pow(base, len))
			return len;
		len++;
	}
}

int strlen(int val, int base){
	return strlen((unsigned long int)(val < 0 ? val * -1 : val), base) + (val < 0 ? 1 : 0);
}

int strlen(long int val, int base){
	return strlen((unsigned long int)(val < 0 ? val * -1 : val), base) + (val < 0 ? 1 : 0);
}

int strsplit(char* line, int* _argc, char*** _argv){
	unsigned int i, j, len, start, arg_len;
	int argc;
	char** argv;


	if(line == 0)
		return -1;

	len = strlen(line);

	/* identify number of arguments within cmdline */
	i = 0;
	argc = 0;

	while(i < len){
		/* skip blanks */
		if(line[i] == ' '){
			while(++i < len && line[i] == ' ');

			if(i == len)
				break;
		}

		while(i < len){
			if(line[i] == '"'){
				/* hande quoted text */
				while(++i < len && line[i] != '"'){
					if(line[i] == '\\' && i + 1 < len && line[i + 1] == '"')
						i++;
				}

				if(++i >= len)
					break;
			}
			else{
				/* handle un-quoted text */
				do{
					if(line[i] == ' ' || line[i] == '"')
						break;

					if(line[i] == '\\' && i + 1 < len && line[i + 1] == '"')
						i++;
				}while(++i < len);
			}

			/* check for end of argument */
			if(line[i] == ' ')
				break;
		}

		if(i > len)
			break;

		argc++;
	}

	/* alloc argv */
	argv = new char*[argc];

	if(argv == 0)
		return -1;

	/* alloc argv[] and assign strings */
	i = 0;
	argc = 0;
	start = 0;
	while(i < len){
		arg_len = 0;

		/* skip blanks */
		if(line[i] == ' '){
			while(++i < len && line[i] == ' ');

			if(i == len)
				break;
		}

		start = i;

		while(i < len){
			if(line[i] == '"'){
				/* handle quoted text */
				while(++i < len && line[i] != '"'){
					if(line[i] == '\\' && i + 1 < len){
						switch(line[i + 1]){
						case '"':
						case 'n':
						case 'r':
						case 't':
						case '\\':
							i++;
							break;

						default:
							break;
						};
					}

					arg_len++;
				}

				if(++i >= len)
					break;
			}
			else{
				/* handle un-quoted text */
				do{
					if(line[i] == ' ' || line[i] == '"')
						break;

					if(line[i] == '\\' && i + 1 < len){
						switch(line[i + 1]){
						case '"':
						case 'n':
						case 'r':
						case 't':
						case '\\':
							i++;
							break;

						default:
							break;
						};
					}

					arg_len++;
				}while(++i < len);
			}

			/* check for end of argument */
			if(line[i] == ' ')
				break;
		}

		if(i > len)
			break;

		/* allocated string */
		argv[argc] = new char[arg_len + 1];

		/* copy content from cmdline to argv[] */
		j = 0;
		for(; start<i; start++){
			if(line[start] == '"')
				continue;

			if(line[start] == '\\'){
				start++;

				switch(line[start]){
				case '"':
					argv[argc][j++] = '\"';
					break;

				case 'n':
					argv[argc][j++] = '\n';
					break;

				case 'r':
					argv[argc][j++] = '\r';
					break;

				case 't':
					argv[argc][j++] = '\t';
					break;

				case '\\':
					argv[argc][j++] = '\\';
					break;
				
				default:
					start--;
					argv[argc][j++] = line[start];
					break;
				};
			}
			else
				argv[argc][j++] = line[start];
		}

		argv[argc][j] = 0;
		argc++;
	}

	*_argv = argv;
	*_argc = argc;

	return 0;
}

char* strescape(char* s, char** _dst, unsigned int* max){
	char* dst;
	char src[strlen(s) + 1];
	unsigned int len;
	unsigned int i, o;
	

	if(s == 0 || _dst == 0)
		return 0;

	len = strlen(s) * 2 + 1;	// at most every char is an escape char
	strcpy(src, s);

	if(*max < len){
		delete [] *_dst;

		*max += len;
		*_dst = new char[*max];
	}

	dst = *_dst;

	if(dst == 0){
		*max = 0;
		return 0;
	}

	for(i=0, o=0; i<len/2; i++, o++){
		switch(src[i]){
		case '\t':
			dst[o] = '\\';
			dst[++o] = 't';
			break;

		case '\n':
			dst[o] = '\\';
			dst[++o] = 'n';
			break;

		case '\r':
			dst[o] = '\\';
			dst[++o] = 'r';
			break;

		case '\"':
			dst[o] = '\\';
			dst[++o] = '\"';
			break;

		case '\\':
			dst[o] = '\\';
			dst[++o] = '\\';
			break;

		default:
			dst[o] = src[i];
		};
	}

	dst[o] = 0;

	return dst;
}

char* strdeescape(char* s){
	unsigned int len;
	unsigned int i, o;
	

	if(s == 0)
		return 0;

	len = strlen(s);

	for(i=0, o=0; i<len; i++, o++){
		if(s[i] == '\\'){
			switch(s[++i]){
			case 't':
				s[o] = '\t';
				break;

			case 'n':
				s[o] = '\n';
				break;

			case 'r':
				s[o] = '\r';
				break;

			case '"':
				s[o] = '\"';
				break;

			case '\\':
				s[o] = '\\';
				break;

			default:
				if(--i != o)
					s[o] = s[i];
			};
		}
		else{
			if(i != o)
				s[o] = s[i];
		}
	}

	s[o] = 0;

	return s;
}

char* itoa(unsigned int v, char** s, unsigned int* max, unsigned int base, bool neg){
	return itoa((unsigned long int)v, s, max, base, neg);
}

char* itoa(int v, char** s, unsigned int* max, unsigned int base){
	return itoa((unsigned long int)(v < 0 ? v * -1 : v), s, max, base, (v < 0 ? true : false));
}

char* itoa(long int v, char** s, unsigned int* max, unsigned int base){
	return itoa((unsigned long int)(v < 0 ? v * -1 : v), s, max, base, (v < 0 ? true : false));
}

char* itoa(unsigned long int v, char** s, unsigned int* max, unsigned int base, bool neg){
	unsigned int len = 0;
	

	if(s == 0)
		return 0;

	len = strlen(v, base) + 1 + (neg ? 1 : 0);

	if(*max < len){
		*max += len;;
		delete [] *s;
		*s = new char[*max];
	}

	if(*s == 0){
		*max = 0;
		return 0;
	}

	switch(base){
	case 10:
		sprintf(*s, "%s%lu", (neg ? "-" : ""), v);
		break;

	case 16:
		sprintf(*s, "%s%lx", (neg ? "-" : ""), v);
		break;

	default:
		ERROR("base %u not supported\n", base);
		return 0;
	}

	return *s;
}

char* stralloc(char* _s, unsigned int len){
	char* s;

	s = new char[len + 1];

	if(s){
		if(_s)	strncpy(s, _s, len);
		else	len = 0;

		s[len] = 0;
	}

	return s;
}

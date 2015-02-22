#include <math.h>
#include <string.h>
#include <stdio.h>


int strlen(int val, int base){
	unsigned int len;


	len = 1;
	while(1){
		if(val < pow(base, len))
			return len;
		len++;
	}
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

char* strescape(char* _s){
	static char* s = 0;
	static unsigned int len = 0;
	unsigned int i, o;
	int x;
	

	if(_s == 0)
		return 0;

	x = strlen(_s) * 2 + 1;	// at most every char is an escape char

	if(len < x){
		len += x;
		delete s;
		s = new char[len];
	}

	if(s == 0)
		return 0;

	for(i=0, o=0; i<x/2; i++, o++){
		switch(_s[i]){
		case '\t':
			s[o] = '\\';
			s[++o] = 't';
			break;

		case '\n':
			s[o] = '\\';
			s[++o] = 'n';
			break;

		case '\r':
			s[o] = '\\';
			s[++o] = 'r';
			break;

		case '\"':
			s[o] = '\\';
			s[++o] = '\"';
			break;

		case '\\':
			s[o] = '\\';
			s[++o] = '\\';
			break;

		default:
			s[o] = _s[i];
		};
	}

	s[o] = 0;

	return s;
}

char* itoa(int v){
	static char* s = 0;
	static unsigned int len = 0;
	int x;
	

	x = strlen(v, 10) + 1;

	if(len < x){
		len += x;
		delete s;
		s = new char[len];
	}

	if(s == 0)
		return 0;


	sprintf(s, "%d", v);
	return s;
}

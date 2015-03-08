#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


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

char* strescape(char* s, char** e, unsigned int* e_max){
	char* t;
	unsigned int len;
	unsigned int i, o;
	

	if(s == 0 || e == 0)
		return 0;

	t = *e;
	len = strlen(s) * 2 + 1;	// at most every char is an escape char

	if(*e_max < len){
		*e_max += len;
		t = new char[*e_max];
		delete *e;
		*e = t;
	}

	if(t == 0){
		*e_max = 0;
		return 0;
	}

	for(i=0, o=0; i<len/2; i++, o++){
		switch(s[i]){
		case '\t':
			t[o] = '\\';
			t[++o] = 't';
			break;

		case '\n':
			t[o] = '\\';
			t[++o] = 'n';
			break;

		case '\r':
			t[o] = '\\';
			t[++o] = 'r';
			break;

		case '\"':
			t[o] = '\\';
			t[++o] = '\"';
			break;

		case '\\':
			t[o] = '\\';
			t[++o] = '\\';
			break;

		default:
			t[o] = s[i];
		};
	}

	t[o] = 0;

	return t;
}

char* itoa(int v, char** s, unsigned int* max){
	unsigned int len = 0;
	

	if(s == 0)
		return 0;

	len = strlen(v, 10) + 1;

	if(*max < len){
		*max += len;;
		delete *s;
		*s = new char[*max];
	}

	if(*s == 0){
		*max = 0;
		return 0;
	}

	sprintf(*s, "%d", v);
	return *s;
}

char* stralloc(char* _s, unsigned int len){
	char* s;

	s = (char*)malloc(len + 1);
	if(s != 0)
		strncpy(s, _s, len + 1);
	return s;
}

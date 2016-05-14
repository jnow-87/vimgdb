#include <common/list.h>
#include <common/log.h>
#include <gdb/gdb.h>
#include <gdb/types.h>
#include <gdb/lexer.lex.h>
#include <gdb/parser.tab.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>


/* global functions */
int main(int argc, char **argv){
	char c, *line;
	unsigned int i, len;
	int fd;


	/* init */
	if(argc < 2){
		printf("usage: %s <gdb-log>\n", argv[0]);
		return 0;
	}

	fd = open(argv[1], O_RDONLY);

	if(fd == -1){
		printf("unable to open file \"%s\"\n", argv[1]);
		return 1;
	}

	if(log::init("/proc/self/fd/1", LOG_LEVEL) != 0)
		return 1;

	gdb = new gdbif;

	i = 0;

	len = 256;
	line = (char*)malloc(len);

	if(line == 0)
		goto err_0;

	/* main loop */
	while(1){
		if(read(fd, &c, 1) == 1){
			// ignore CR to avoid issues when printing the string
			if(c == '\r')
				continue;

			line[i++] = c;

			if(i >= len){
				len *= 2;
				line = (char*)realloc(line, len);

				if(line == 0)
					goto err_0;
			}

			// check for end of gdb line, a simple newline as separator
			// doesn't work, since the parse would try to parse the line,
			// detecting a syntax error
			if((i >= 6 && strncmp(line + i - 6, "(gdb)\n", 6) == 0) ||
			   (i >= 7 && strncmp(line + i - 7, "(gdb) \n", 7) == 0)
			  ){
				line[i] = 0;

				printf("\n----------------------------------------\n");
				printf("parse: \"%s\"\n\n", line);

				i = gdbparse(line, gdb);
				gdblex_destroy();

				printf("parser return value: %d\n", i);
				printf("----------------------------------------\n");

				if(i != 0)
					break;

				i = 0;
			}
		}
		else
			break;
	}

	/* cleanup */
	free(line);

err_0:
	delete gdb;
	close(fd);
	log::cleanup();

	return 0;
}

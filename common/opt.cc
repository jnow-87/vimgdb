#include <common/defaults.h>
#include <common/string.h>
#include <common/opt.h>
#include <version.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


opt_t opt;


int opt_parse(int argc, char **argv){
	int i, gdb_argc;
	char **gdb_argv;


	if(argc < 2){
		printf(
			"usage: %s [options] <vim-CWD>\n"
			"options:\n"
			"    -d        start as daemon\n"
			"    -c <cmd>  gdb command line\n"
			"    -l <file> log file\n\n"
			"version info:\n" VERSION "\n"
			, argv[0]
		);

		return -1;
	}

	gdb_argc = 1;
	gdb_argv = 0;

	/* init opt */
	opt.prg_name = argv[0];
	opt.vim_cwd = argv[argc - 1];
	opt.gdb_argv = 0;
	opt.log_file = 0;

	if(opt.vim_cwd[strlen(opt.vim_cwd) - 1] == '/')
		opt.vim_cwd[strlen(opt.vim_cwd) - 1] = 0;

	/* parse arguments */
	for(i=1; i<argc-1; i++){
		switch(argv[i][1]){
		case 'd':
			/* daemonise */
			if(daemon(1, 0) != 0)
				return -1;
			break;

		case 'c':
			if(++i >= argc - 1){
				printf("invalid argument to -c, expected gdb command line\n");
				return -1;
			}

			if(strsplit(argv[i], &gdb_argc, &gdb_argv) != 0)
				return -1;
			break;

		case 'l':
			if(++i >= argc - 1){
				printf("invalid argument to -l, expected log file\n");
				return -1;
			}

			opt.log_file = argv[i];
			break;

		default:
			printf("unsupported option \"%s\"\n", argv[i]);
			return -1;
		};
	}

	/* init gdb arguments */
	opt.gdb_argv = new char*[3 + gdb_argc];
	opt.gdb_argv[0] = gdb_argv ? gdb_argv[0] : (char*)GDB_CMD;

	for(i=1; i<gdb_argc; i++)
		opt.gdb_argv[i] = gdb_argv[i];

	opt.gdb_argv[i++] = (char*)GDB_ARGV1;
	opt.gdb_argv[i++] = (char*)GDB_ARGV2;
	opt.gdb_argv[i] = 0;

	return 0;
}

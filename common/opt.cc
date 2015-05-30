#include <common/opt.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>


opt_t opt;


int opt_parse(int argc, char** argv){
	int i;
	FILE* fp;


	if(argc < 2){
		printf(
			"usage: %s [options] <vim-CWD>\n"
			"options:\n"
			"    -d     start as daemon\n"
			, argv[0]
		);

		return -1;
	}

	opt.prg_name = argv[0];
	opt.vim_cwd = argv[argc - 1];

	if(opt.vim_cwd[strlen(opt.vim_cwd) - 1] == '/')
		opt.vim_cwd[strlen(opt.vim_cwd) - 1] = 0;

	for(i=1; i<argc-1; i++){
		switch(argv[i][1]){
		case 'd':
			/* daemonise */
			if(daemon(1, 0) != 0)
				return -1;

			/* write pid file */
			fp = fopen(PID_FILE, "w");

			if(fp == 0)
				return -1;

			fprintf(fp, "%d", getpid());
			fclose(fp);

			break;

		default:
			printf("unsupported option \"%s\"\n", argv[i]);
			break;
		};
	}

	return 0;
}

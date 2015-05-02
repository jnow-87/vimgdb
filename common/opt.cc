#include <common/opt.h>
#include <stdio.h>
#include <string.h>


opt_t opt;

int opt_parse(int argc, char** argv){
	if(argc < 2){
		printf("usage: %s <vim-CWD>\n", argv[0]);
		return -1;
	}

	opt.prg_name = argv[0];
	opt.vim_cwd = argv[1];

	if(opt.vim_cwd[strlen(opt.vim_cwd) - 1] == '/')
		opt.vim_cwd[strlen(opt.vim_cwd) - 1] = 0;

	return 0;
}

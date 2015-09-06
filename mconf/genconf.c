/*
 * Copyright (C) 2002 Roman Zippel <zippel@linux-m68k.org>
 * Released under the terms of the GNU GPL v2.0.
 *
 * Introduced single menu mode (show all sub-menus in one large tree).
 * 2002-11-06 Petr Baudis <pasky@ucw.cz>
 *
 * i18n, 2005, Arnaldo Carvalho de Melo <acme@conectiva.com.br>
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <locale.h>

#include "lkc.h"


int main(int argc, char **argv)
{
	if(argc < 4){
		printf("usage: %s <Kconfig> <genheader> <split-path>\n", argv[0]);
		return 1;
	}

	conf_parse(argv[1]);
	conf_read(NULL);

	conf_write_autoconf(argv[2], argv[3]);
	return 0;
}

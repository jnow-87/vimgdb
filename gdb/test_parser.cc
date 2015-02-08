#include <common/xmalloc.h>
#include <common/list.h>
#include <common/log.h>
#include <gdb/gdb.h>
#include <gdb/value.h>
#include <gdb/result.h>
#include <gdb/parser.tab.h>
#include <gdb/lexer.lex.h>
#include <stdio.h>
#include "config.h"


/* global variables */
char* line;


/* global functions */
int main(int argc, char** argv){
	gdb_if* gdb;
//	char str[] = "^error,msg=\"Undefined target command: \\\"gdbctrl\\\".  Try \\\"help target\\\".\"\n(gdb)\n";
//	char str[] = "&\"target gdbctrl\\n\"\n&\"Undefined target command: \\\"gdbctrl\\\".  Try \\\"help target\\\".\n\n\"\n^error,msg=\"Undefined target command: \\\"gdbctrl\\\".  Try \\\"help target\\\".\"\n(gdb)\n";
	char str[] = "=breakpoint-created,bkpt=[number=[\"1\",\"2\"],type=\"breakpoint\",thread-groups=[\"i1\",\"i2\",\"i3\"]]\n(gdb)\n";


	// logging
	if(log::init("/proc/self/fd/1", LOG_LEVEL) != 0)
		return 1;

#if 0
	xmalloc_init();

	result_t *res;
	value_t *val;
	char* s;


	// value
	s = malloc(strlen("lst1_1") + 1); strncpy(s, "lst1_1", strlen("lst1_1") + 1);
	val = gdb_value_create(CONST, s);

	s = malloc(strlen("lst1_1") + 1); strncpy(s, "lst1_1", strlen("lst1_1") + 1);
	gdb_value_add(val, gdb_value_create(CONST, s));

	s = malloc(strlen("lst1_1") + 1); strncpy(s, "lst1_1", strlen("lst1_1") + 1);
	gdb_value_add(val, gdb_value_create(CONST, s));

	s = malloc(strlen("lst1_1") + 1); strncpy(s, "lst1_1", strlen("lst1_1") + 1);
	gdb_value_add(val, gdb_value_create(CONST, s));

	// res
	s = malloc(strlen("res0") + 1); strncpy(s, "res0", strlen("res0") + 1);
	res = gdb_result_create(s, val);

	// value
	s = malloc(strlen("lst1_1") + 1); strncpy(s, "lst1_1", strlen("lst1_1") + 1);
	val = gdb_value_create(CONST, s);
	s = malloc(strlen("lst1_1") + 1); strncpy(s, "lst1_1", strlen("lst1_1") + 1);
	gdb_value_add(val, gdb_value_create(CONST, s));
	s = malloc(strlen("lst1_1") + 1); strncpy(s, "lst1_1", strlen("lst1_1") + 1);
	gdb_value_add(val, gdb_value_create(CONST, s));
	s = malloc(strlen("lst1_1") + 1); strncpy(s, "lst1_1", strlen("lst1_1") + 1);
	gdb_value_add(val, gdb_value_create(CONST, s));

	// res
	s = malloc(strlen("res1") + 1); strncpy(s, "res1", strlen("res1") + 1);
	gdb_result_add(res, gdb_result_create(s, val));

	// value
	s = malloc(strlen("lst1_1") + 1); strncpy(s, "lst1_1", strlen("lst1_1") + 1);
	val = gdb_value_create(CONST, s);
	s = malloc(strlen("lst1_1") + 1); strncpy(s, "lst1_1", strlen("lst1_1") + 1);
	gdb_value_add(val, gdb_value_create(CONST, s));
	s = malloc(strlen("lst1_1") + 1); strncpy(s, "lst1_1", strlen("lst1_1") + 1);
	gdb_value_add(val, gdb_value_create(CONST, s));
	s = malloc(strlen("lst1_1") + 1); strncpy(s, "lst1_1", strlen("lst1_1") + 1);
	gdb_value_add(val, gdb_value_create(CONST, s));

	// res
	s = malloc(strlen("res1") + 1); strncpy(s, "res1", strlen("res1") + 1);
	gdb_result_add(res, gdb_result_create(s, val));

	gdb_result_print(res);
	gdb_result_free(res);
	xmalloc_eval();

	return 0;
#else // 0

	xmalloc_init();

	gdb = new gdb_if;

	if(argc < 2)
		line = str;
	else
		line = argv[1];

	USER("%s\n", str);
	gdb_scan_string(line);
	printf("parser result: %d\n", gdbparse(gdb));

	delete gdb;

	xmalloc_eval();
	return 0;
#endif // 0
}

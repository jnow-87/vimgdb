%define api.prefix {gdb}
%locations

%{
	#include <stdio.h>
	#include <gdbparser.lex.h>
	#include <gdb_value.h>
	#include <gdb_result.h>


	int gdberror(const char* s);
%}

%union{
	char* sptr;
	unsigned int num;

	result_t* result;
	value_t* value;
}

/* terminals */
%token NEWLINE
%token GDB
%token ASYNC_CLASS
%token RESULT_CLASS
%token <sptr> STRING
%token <num> NUMBER

/* non-terminals */
%type <result> async-record
%type <result> async-output
%type <sptr> stream-record
%type <result> result-record
%type <result> result
%type <result> result-list
%type <value> value
%type <value> value-list
%type <value> const
%type <value> tuple
%type <value> list
%type <sptr> variable
%type <num>token

%%


output :	out-of-band-record result-record GDB NEWLINE			{ printf("output\n"); };

/* out-of-band-record */
out-of-band-record :	%empty										{ printf("oob-empty\n"); }
				   |	out-of-band-record async-record				{ printf("oob-async\n"); }
				   |	out-of-band-record stream-record			{ printf("oob-stream\n"); }
				   ;

async-record :			token '*' async-output NEWLINE				{}		/* exec-async-output */
			 |			token '+' async-output NEWLINE				{}		/* status-async-output */
			 |			token '=' async-output NEWLINE				{}		/* notify-async-output */
			 ;

async-output :			ASYNC_CLASS									{  }
			 |			ASYNC_CLASS ',' result-list					{ gdb_result_print($3); gdb_result_free($3); }
			 ;

stream-record :			'~' '"' STRING '"' NEWLINE					{ printf("console stream: \"%s\"\n", $3); }		/* console-stream-output */
			  |			'@' '"' STRING '"' NEWLINE					{ printf("target system stream: \"%s\"\n", $3); }		/* target-system-output */
			  |			'&' '"' STRING '"' NEWLINE					{ printf("log stream: \"%s\"\n", $3); }	/* log-stream-output */
			  ;


/* result-record */
result-record :		%empty											{}
			  |		token '^' RESULT_CLASS NEWLINE					{}
			  |		token '^' RESULT_CLASS ',' result-list NEWLINE	{}
			  ;


/* common */
result :			variable '=' value								{ $$ = gdb_result_create($1, $3); };
result-list :		result											{ $$ = $1; }
		    |		result-list ',' result							{ gdb_result_add($1, $3); $$ = $1; }
	   		;

value :				const											{ $$ = $1; }
	  |				tuple											{ $$ = $1; }
	  |				list											{ $$ = $1; }
	  ;

value-list :		value											{ $$ = $1; }
		   |		value-list ',' value							{ gdb_value_add($1, $3); $$ = $1; }
		   ;

const :				'"' STRING '"'									{ $$ = gdb_value_create(CONST, $2); };
tuple :				'{' '}'											{ $$ = gdb_value_create(EMPTY, 0); }
	  |				'{' result-list '}'								{ $$ = gdb_value_create(RESULT_LIST, $2); }
	  ;

list :				'[' ']'											{ $$ = gdb_value_create(EMPTY, 0); }
	 |				'[' result-list ']'								{ $$ = gdb_value_create(RESULT_LIST, $2); }
	 |				'[' value-list ']'								{ $$ = gdb_value_create(VALUE_LIST, $2); }
	 ;

variable :			STRING											{ $$ = $1; };

token :				%empty											{ $$ = 0; }
	  |				NUMBER											{ $$ = $1; }
	  ;


%%


#ifdef TEST

#include "xmalloc.h"


char* line;
unsigned int token = 1;


#include "list.h"


int main(int argc, char** argv){
//	char str[] = "^error,msg=\"Undefined target command: \\\"gdbctrl\\\".  Try \\\"help target\\\".\"\n(gdb)\n";
//	char str[] = "&\"target gdbctrl\\n\"\n&\"Undefined target command: \\\"gdbctrl\\\".  Try \\\"help target\\\".\n\n\"\n^error,msg=\"Undefined target command: \\\"gdbctrl\\\".  Try \\\"help target\\\".\"\n(gdb)\n";
	char str[] = "=breakpoint-created,bkpt=[number=[\"1\",\"2\"],type=\"breakpoint\",groups=[\"i1\",\"i2\",\"i3\"]]\n(gdb)\n";

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

	if(argc < 2)
		line = str;
	else
		line = argv[1];

	printf("%s\n", str);
	gdb_scan_string(line);
	gdbparse();

	xmalloc_eval();
	return 0;
#endif // 0
}

#endif // TEST

int gdberror(const char* s){
	char tmp;
	unsigned int i;


	printf("%s at token \"%s\" columns (%d - %d)\n", s, gdbtext, gdblloc.first_column, gdblloc.last_column);

#ifdef TEST
	tmp = line[gdblloc.last_column];
	line[gdblloc.last_column] = 0;
	printf("%s\n", line);
	line[gdblloc.last_column] = tmp;
	for(i=1; i<gdblloc.last_column; i++)
		printf(" ");
	printf("^\n");
#endif // TEST

	return 0;
}

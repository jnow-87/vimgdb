%define api.prefix {gdb}
%locations

%{
	#include <common/log.h>
	#include <gdb/gdb.h>
	#include <gdb/value.h>
	#include <gdb/result.h>
	#include <gdb/lexer.lex.h>


	int gdberror(gdb_if* gdb, const char* s);
%}

%union{
	char* sptr;
	unsigned int num;

	result_t* result;
	value_t* value;
	async_class_t aclass;
	result_class_t rclass;

	struct{
		async_class_t aclass;
		result_t* result;
	} async_out;
}


%parse-param { gdb_if* gdb }

/* terminals */
%token NEWLINE
%token GDB
%token <aclass> ASYNC_CLASS
%token <rclass> RESULT_CLASS
%token <sptr> STRING
%token <num> NUMBER

/* non-terminals */
%type <async_out> async-output
%type <result> result
%type <result> result-list
%type <value> value
%type <value> value-list
%type <value> const
%type <value> tuple
%type <value> list
%type <sptr> variable
%type <num> token


%%


output :	out-of-band-record result-record GDB NEWLINE			{ TEST("output\n"); };


/* out-of-band-record */
out-of-band-record :	%empty										{ TEST("oob-empty\n"); }
				   |	out-of-band-record async-record				{ TEST("oob-async\n"); }
				   |	out-of-band-record stream-record			{ TEST("oob-stream\n"); }
				   ;

async-record :			token '*' async-output NEWLINE				{ gdb->mi_proc_async($3.aclass, $1, $3.result); }		/* exec-async-output */
			 |			token '+' async-output NEWLINE				{ gdb->mi_proc_async($3.aclass, $1, $3.result); }		/* status-async-output */
			 |			token '=' async-output NEWLINE				{ gdb->mi_proc_async($3.aclass, $1, $3.result); }		/* notify-async-output */
			 ;

async-output :			ASYNC_CLASS									{ $$.aclass = $1; $$.result = 0; }
			 |			ASYNC_CLASS ',' result-list					{ $$.aclass = $1; $$.result = $3; }
			 ;

stream-record :			'~' '"' STRING '"' NEWLINE					{ gdb->mi_proc_stream(SC_CONSOLE, $3); }	/* console-stream-output */
			  |			'@' '"' STRING '"' NEWLINE					{ gdb->mi_proc_stream(SC_CONSOLE, $3); }	/* target-system-output */
			  |			'&' '"' STRING '"' NEWLINE					{ gdb->mi_proc_stream(SC_CONSOLE, $3); }	/* log-stream-output */
			  ;


/* result-record */
result-record :		%empty											{ TEST("result empty\n"); }
			  |		token '^' RESULT_CLASS NEWLINE					{ gdb->mi_proc_result($3, $1, 0); }
			  |		token '^' RESULT_CLASS ',' result-list NEWLINE	{ gdb->mi_proc_result($3, $1, $5); }
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

int gdberror(gdb_if* gdb, const char* s){
	ERROR("%s at token \"%s\" columns (%d - %d)\n", s, gdbtext, gdblloc.first_column, gdblloc.last_column);
	return 0;
}

%define api.prefix {gdb}
%locations

%{
	#include <common/log.h>
	#include <gdb/value.h>
	#include <gdb/result.h>
	#include <lexer.lex.h>


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


output :	out-of-band-record result-record GDB NEWLINE			{ DEBUG("output\n"); };

/* out-of-band-record */
out-of-band-record :	%empty										{ DEBUG("oob-empty\n"); }
				   |	out-of-band-record async-record				{ DEBUG("oob-async\n"); }
				   |	out-of-band-record stream-record			{ DEBUG("oob-stream\n"); }
				   ;

async-record :			token '*' async-output NEWLINE				{}		/* exec-async-output */
			 |			token '+' async-output NEWLINE				{}		/* status-async-output */
			 |			token '=' async-output NEWLINE				{}		/* notify-async-output */
			 ;

async-output :			ASYNC_CLASS									{  }
			 |			ASYNC_CLASS ',' result-list					{ gdb_result_print($3); gdb_result_free($3); }
			 ;

stream-record :			'~' '"' STRING '"' NEWLINE					{ DEBUG("console stream: \"%s\"\n", $3); }		/* console-stream-output */
			  |			'@' '"' STRING '"' NEWLINE					{ DEBUG("target system stream: \"%s\"\n", $3); }		/* target-system-output */
			  |			'&' '"' STRING '"' NEWLINE					{ DEBUG("log stream: \"%s\"\n", $3); }	/* log-stream-output */
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

int gdberror(const char* s){
	ERROR("%s at token \"%s\" columns (%d - %d)\n", s, gdbtext, gdblloc.first_column, gdblloc.last_column);
	return 0;
}

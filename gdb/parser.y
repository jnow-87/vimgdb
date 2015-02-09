%define api.prefix {gdb}
%locations

%{
	#include <common/log.h>
	#include <gdb/gdb.h>
	#include <gdb/value.h>
	#include <gdb/result.h>
	#include <gdb/variable.h>
	#include <gdb/lexer.lex.h>


	int gdberror(gdbif* gdb, const char* s);
%}

%union{
	char* sptr;
	unsigned int num;

	result_t* result;
	value_t* value;
	result_class_t rclass;
	variable_t variable;

	struct{
		result_class_t rclass;
		result_t* result;
	} record;
}


%parse-param { gdbif* gdb }

%initial-action
{
	xxxx:;
}

/* terminals */
%token NEWLINE
%token GDB
%token <rclass> RESULT_CLASS
%token <sptr> STRING
%token <num> NUMBER
%token <variable> VARIABLE

/* non-terminals */
%type <record> record
%type <result> result
%type <result> result-list
%type <value> value
%type <value> value-list
%type <value> const
%type <value> tuple
%type <value> list
%type <num> token


%%


output :	out-of-band-record GDB NEWLINE			{ DEBUG("gdb parser: reduced to output\n"); };


/* out-of-band-record */
out-of-band-record :	%empty										{ DEBUG("gdb parser: reduced to empty out-of-band-record\n"); }
				   |	out-of-band-record async-record				{ DEBUG("gdb parser: reduced to out-of-band-record with async-record\n"); }
				   |	out-of-band-record stream-record			{ DEBUG("gdb parser: reduced to out-of-band-record with stream-record\n"); }
				   ;

async-record :			token '*' record NEWLINE					{ gdb->mi_proc_async($3.rclass, $1, $3.result); }		/* exec-async-output */
			 |			token '+' record NEWLINE					{ gdb->mi_proc_async($3.rclass, $1, $3.result); }		/* status-async-output */
			 |			token '=' record NEWLINE					{ gdb->mi_proc_async($3.rclass, $1, $3.result); }		/* notify-async-output */
			 |			token '^' record NEWLINE					{ gdb->mi_proc_result($3.rclass, $1, $3.result); }		/* result-record */
			 ;

record :				RESULT_CLASS								{ $$.rclass = $1, $$.result = 0; }
	   |				RESULT_CLASS ',' result-list				{ $$.rclass = $1, $$.result = $3; }
	   ;

stream-record :			'~' '"' STRING '"' NEWLINE					{ gdb->mi_proc_stream(SC_CONSOLE, $3); }	/* console-stream-output */
			  |			'@' '"' STRING '"' NEWLINE					{ gdb->mi_proc_stream(SC_CONSOLE, $3); }	/* target-system-output */
			  |			'&' '"' STRING '"' NEWLINE					{ gdb->mi_proc_stream(SC_CONSOLE, $3); }	/* log-stream-output */
			  ;

/* common */
result :			VARIABLE '=' value								{ $$ = gdb_result_create($1.name, $1.id, $3); };
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

token :				%empty											{ $$ = 0; }
	  |				NUMBER											{ $$ = $1; }
	  ;


%%


int gdberror(gdbif* gdb, const char* s){
	ERROR("%s at token \"%s\" columns (%d - %d)\n", s, gdbtext, gdblloc.first_column, gdblloc.last_column);
	return 0;
}

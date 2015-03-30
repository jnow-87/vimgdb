%define api.prefix {gdb}
%locations

%{
	#include <common/log.h>
	#include <common/string.h>
	#include <gdb/gdb.h>
	#include <gdb/value.h>
	#include <gdb/result.h>
	#include <gdb/identifier.h>
	#include <gdb/lexer.lex.h>


	int gdberror(char* line, gdbif* gdb, const char* s);
%}

%union{
	char* sptr;
	unsigned int num;

	gdb_result_t* result;
	gdb_value_t* value;
	gdb_result_class_t rclass;
	const gdb_id_t* var_id;

	struct{
		gdb_result_class_t rclass;
		gdb_result_t* result;
	} record;
}


%parse-param { char* line }
%parse-param { gdbif* gdb }

%initial-action{
	gdb_scan_string(line);
}

/* terminals */
%token NEWLINE
%token GDB
%token <rclass> RESULT_CLASS
%token <sptr> STRING
%token <num> NUMBER
%token <var_id> IDENTIFIER

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


output :	out-of-band-record GDB NEWLINE			{ };


/* out-of-band-record */
out-of-band-record :	%empty										{ }
				   |	out-of-band-record async-record				{ }
				   |	out-of-band-record stream-record			{ }
				   ;

async-record :			token '*' record NEWLINE					{ gdb->mi_proc_async($3.rclass, $1, $3.result); }		/* exec-async-output */
			 |			token '+' record NEWLINE					{ gdb->mi_proc_async($3.rclass, $1, $3.result); }		/* status-async-output */
			 |			token '=' record NEWLINE					{ gdb->mi_proc_async($3.rclass, $1, $3.result); }		/* notify-async-output */
			 |			token '^' record NEWLINE					{ gdb->mi_proc_result($3.rclass, $1, $3.result); }		/* result-record */
			 |			error NEWLINE								{ }														/* allow continued parsing after error */
			 ;

record :				RESULT_CLASS								{ $$.rclass = $1, $$.result = 0; }
	   |				RESULT_CLASS ',' result-list				{ $$.rclass = $1, $$.result = $3; }
	   ;

stream-record :			'~' '"' STRING '"' NEWLINE					{ gdb->mi_proc_stream(SC_CONSOLE, $3); }	/* console-stream-output */
			  |			'@' '"' STRING '"' NEWLINE					{ gdb->mi_proc_stream(SC_CONSOLE, $3); }	/* target-system-output */
			  |			'&' '"' STRING '"' NEWLINE					{ gdb->mi_proc_stream(SC_CONSOLE, $3); }	/* log-stream-output */
			  ;

/* common */
result :			IDENTIFIER '=' value							{ $$ = gdb_result_create($1->name, $1->id, $3); };
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

const :             '"' '"'                                         { $$ = gdb_value_create(VT_CONST, stralloc((char*)"", 0)); };
const :				'"' STRING '"'									{ $$ = gdb_value_create(VT_CONST, $2); };
tuple :				'{' '}'											{ $$ = gdb_value_create(VT_EMPTY, 0); }
	  |				'{' result-list '}'								{ $$ = gdb_value_create(VT_RESULT_LIST, $2); }
	  ;

list :				'[' ']'											{ $$ = gdb_value_create(VT_EMPTY, 0); }
	 |				'[' result-list ']'								{ $$ = gdb_value_create(VT_RESULT_LIST, $2); }
	 |				'[' value-list ']'								{ $$ = gdb_value_create(VT_VALUE_LIST, $2); }
	 ;

token :				%empty											{ $$ = 0; }
	  |				NUMBER											{ $$ = $1; }
	  ;


%%


int gdberror(char* line, gdbif* gdb, const char* s){
	USER("gdbparse: %s at token \"%s\" columns (%d - %d)\n", s, gdbtext, gdblloc.first_column, gdblloc.last_column);
	return 0;
}

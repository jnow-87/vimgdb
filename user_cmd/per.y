%define api.prefix {per}
%locations

%{
	#include <common/log.h>
	#include <common/list.h>
	#include <user_cmd/pertypes.h>
	#include <user_cmd/per.lex.h>


	/* macros */
	// extended error messages
	#define YYERROR_VERBOSE


	/* prototypes */
	int pererror(FILE* file, per_range_t** rlst, const char* s);
%}

%union{
char* sptr;
long long int i;

per_range_t* range;
per_register_t* reg;
per_bits_t* bits;
}


%parse-param { FILE* file }
%parse-param { per_range_t** rlst }

%initial-action{
	if(!rlst)
		return -1;

	*rlst = 0;

	perrestart(file);
}

/* terminals */
%token RANGE
%token REGISTER
%token BITS
%token END
%token <sptr> STRING
%token <i> INT

/* non-terminals */
%type <range> range
%type <reg> register
%type <bits> bits


%%


/* start */
start :		range END													{ *rlst = $1; return 0; }
	  ;

range :		%empty														{ $$ = 0; }
	  |		range RANGE STRING INT INT '=' '{' nl register '}' nl		{ $$ = $1; list_add_tail(&$$, new per_range_t($3, $4, $5, $9)); }
	  ;

register :	%empty														{ $$ = 0; }
		 |	register REGISTER STRING INT INT '=' '{' nl bits '}' nl		{ $$ = $1; list_add_tail(&$$, new per_register_t($3, $4, $5, $9)); }
		 ;

bits :		%empty														{ $$ = 0; }
	 |		bits BITS STRING INT INT nl									{ $$ = $1; list_add_tail(&$$, new per_bits_t($3, $4, $5)); }
	 ;

nl :		%empty														{ }
   |		'\n'														{ }
   ;


%%


int pererror(FILE* file, per_range_t** rlst, const char* s){
	USER("perparse: %s at token \"%s\" line %d, columns (%d - %d)\n", s, pertext, perlloc.first_line, perlloc.first_column, perlloc.last_column);
	return 0;
}

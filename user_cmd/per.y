%define api.prefix {per}
%locations

%{
	#include <common/log.h>
	#include <common/list.h>
	#include <user_cmd/pertypes.h>
	#include <user_cmd/per.lex.h>


	/* macros */
	/* global variables */
	unsigned int per_col,
				 per_line;

	// extended error messages
	#define YYERROR_VERBOSE


	/* prototypes */
	int pererror(FILE* file, per_range_t** rlst, const char* s);
%}

%union{
char* sptr;
long long int i;

per_section_t* sec;
per_range_t* range;
per_register_t* reg;
per_reg_opt_t ropt;
per_bits_t* bits;
}


%parse-param { FILE* file }
%parse-param { per_range_t** rlst }

%initial-action{
	if(!rlst)
		return -1;

	per_col = 0;
	per_line = 1;
	*rlst = 0;

	perrestart(file);
}

/* terminals */
%token SECTION
%token RANGE
%token REGISTER
%token BITS
%token EMPTYLINE
%token HEADLINE
%token SWAP
%token END
%token <sptr> STRING
%token <i> INT

/* non-terminals */
%type <sec> section
%type <range> range
%type <reg> register
%type <ropt> register-opt
%type <bits> bits


%%


/* start */
start :		range END															{ *rlst = $1; return 0; }
	  ;

range :		%empty																{ $$ = 0; }
	  |		range RANGE INT INT '{' section '}'									{ $$ = $1; list_add_tail(&$$, new per_range_t((void*)$3, $4, $6)); }
	  ;

section :	%empty																{ $$ = 0; }
		|	section SECTION STRING '{' register '}'								{ $$ = $1; list_add_tail(&$$, new per_section_t($3, $5)); }
		;

register :	%empty																{ $$ = 0; }
		 |	register REGISTER STRING STRING INT INT register-opt '{' bits '}'	{ $$ = $1; list_add_tail(&$$, new per_register_t($3, $4, $5, $6, $7, $9)); }
		 |	register EMPTYLINE													{ $$ = $1; list_add_tail(&$$, new per_register_t(0, 0, 0, 0, REG_NONE, 0)); }
		 |	register HEADLINE STRING											{ $$ = $1; list_add_tail(&$$, new per_register_t($3, 0, 0, 0, REG_NONE, 0)); }
		 ;

register-opt :		%empty														{ $$ = REG_NONE; }
	 		 |		register-opt SWAP											{ $$ = (per_reg_opt_t)($1 | REG_SWAP); }
	 		 ;

bits :		%empty																{ $$ = 0; }
	 |		bits BITS STRING INT INT											{ $$ = $1; list_add_tail(&$$, new per_bits_t($3, $4, $5)); }
	 ;

%%


int pererror(FILE* file, per_range_t** rlst, const char* s){
	USER("perparse: %s at token \"%s\" line %d, columns (%d - %d)\n", s, pertext, perlloc.first_line, perlloc.first_column, perlloc.last_column);
	return 0;
}

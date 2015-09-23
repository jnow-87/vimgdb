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
	int pererror(FILE* file, per_section_t** rlst, const char* s);
%}

%union{
char* sptr;
long long int i;

per_section_t* sec;
per_range_t* range;
per_register_t* reg;
per_bits_t* bits;
}


%parse-param { FILE* file }
%parse-param { per_section_t** rlst }

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
%token END
%token <sptr> STRING
%token <i> INT

/* non-terminals */
%type <sec> section
%type <range> range
%type <reg> register
%type <bits> bits


%%


/* start */
start :		section END												{ *rlst = $1; return 0; }
	  ;

section :	%empty													{ $$ = 0; }
		|	section SECTION STRING '{' range '}'					{ $$ = $1; list_add_tail(&$$, new per_section_t($3, $5)); }
		;

range :		%empty													{ $$ = 0; }
	  |		range RANGE INT INT '{' register '}'					{ $$ = $1; list_add_tail(&$$, new per_range_t(0, (void*)$3, $4, $6)); }
	  |		range EMPTYLINE											{ $$ = $1; list_add_tail(&$$, new per_range_t(0, 0, 0, 0)); }
	  |		range HEADLINE STRING									{ $$ = $1; list_add_tail(&$$, new per_range_t($3, 0, 0, 0)); }
	  ;

register :	%empty													{ $$ = 0; }
		 |	register REGISTER STRING STRING INT INT '{' bits '}'	{ $$ = $1; list_add_tail(&$$, new per_register_t($3, $4, $5, $6, $8)); }
		 |	register EMPTYLINE										{ $$ = $1; list_add_tail(&$$, new per_register_t(0, 0, 0, 0, 0)); }
		 |	register HEADLINE STRING								{ $$ = $1; list_add_tail(&$$, new per_register_t($3, 0, 0, 0, 0)); }
		 ;

bits :		%empty													{ $$ = 0; }
	 |		bits BITS STRING INT INT								{ $$ = $1; list_add_tail(&$$, new per_bits_t($3, $4, $5)); }
	 ;

%%


int pererror(FILE* file, per_section_t** rlst, const char* s){
	USER("perparse: %s at token \"%s\" line %d, columns (%d - %d)\n", s, pertext, perlloc.first_line, perlloc.first_column, perlloc.last_column);
	return 0;
}

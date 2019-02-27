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
	int pererror(FILE *file, per_range_t **rlst, per_prop_t *props, const char *s);
%}

%union{
	char *sptr;
	long long int i;

	per_section_t *sec;
	per_range_t *range;
	per_register_t *reg;
	per_reg_opt_t ropt;
	per_bits_t *bits;
}


%parse-param { FILE *file }
%parse-param { per_range_t **rlst }
%parse-param { per_prop_t *props }

%initial-action{
	if(!rlst)
		return -1;

	per_col = 0;
	per_line = 1;
	*rlst = 0;

	memset(props, 0x0, sizeof(per_prop_t));

	perrestart(file);
}

/* terminals */
%token PROPERTY
%token SECTION
%token RANGE
%token REGISTER
%token BITS
%token EMPTYLINE
%token HEADLINE
%token ENDIAN_LITTLE
%token ENDIAN_BIG
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
start :		%empty																{ }
	  |		start property														{ }
	  |		start range															{ list_add_tail(rlst, $2); }
	  ;

/* property */
property :	PROPERTY ENDIAN_LITTLE												{ props->endian = END_LITTLE; }
		 |	PROPERTY ENDIAN_BIG													{ props->endian = END_BIG; }
		 ;

/* range */
range :		RANGE INT INT '{' section '}'										{ $$ = new per_range_t((void*)$2, $3, $5); }
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
	 		 |		register-opt ENDIAN_LITTLE									{ $$ = (per_reg_opt_t)($1 | REG_END_LITTLE); }
	 		 |		register-opt ENDIAN_BIG										{ $$ = (per_reg_opt_t)($1 | REG_END_BIG); }
	 		 ;

bits :		%empty																{ $$ = 0; }
	 |		bits BITS STRING INT INT											{ $$ = $1; list_add_tail(&$$, new per_bits_t($3, $4, $5)); }
	 ;

%%


int pererror(FILE *file, per_range_t **rlst, per_prop_t *props, const char *s){
	USER("perparse: %s at token \"%s\" line %d, columns (%d - %d)\n", s, pertext, perlloc.first_line, perlloc.first_column, perlloc.last_column);
	return 0;
}

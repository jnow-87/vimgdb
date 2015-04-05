%define api.prefix {vim}
%locations

%{
	#include <common/log.h>
	#include <gui/vim/vimui.h>
	#include <gui/vim/event.h>
	#include <gui/vim/result.h>
	#include <lexer.lex.h>


	/* macros */
	// extended error messages
	#define YYERROR_VERBOSE


	/* prototypes */
	int vimerror(char* line, vimui* vim, const char* s);
%}

%union{
	char* sptr;
	int num;

	const vim_event_t* event;

	vim_result_t* result;
}

%parse-param { char* line }
%parse-param { vimui* vim }

%initial-action{
	vim_scan_string(line);
}


/* terminals */
%token NEWLINE
%token <sptr> STRING
%token <num> NUMBER
%token <event> EVENT

/* non-terminals */
%type <result> arg-list


%%


line :	NUMBER arg-list NEWLINE											{ return vim->reply($1, $2); }			/* reply */
	 |	NUMBER ':' EVENT '=' NUMBER arg-list NEWLINE					{ return vim->event($1, $5, $3, $6); }	/* event */
	 |	EVENT arg-list NEWLINE											{ return vim->event(0, 0, $1, $2); }	/* special messages */
	 ;

arg-list :	%empty														{ $$ = 0; }
		 |	arg-list ' ' NUMBER											{ $$ = vim_result_add($1, vim_result_create(RT_INT, (void*)&($3))); }
		 |	arg-list ' ' NUMBER '/' NUMBER								{ $$ = vim_result_add($1, vim_result_create(RT_LINENUM, (void*)&($3), (void*)&($5))); }
		 |	arg-list ' ' STRING			 								{ $$ = vim_result_add($1, vim_result_create(RT_STR, (void*)$3)); }
		 |	arg-list ' ' '!' STRING			 							{ $$ = vim_result_add($1, vim_result_create(RT_STR, (void*)$4)); }
		 |	arg-list ' ' '"' STRING '"' 								{ $$ = vim_result_add($1, vim_result_create(RT_STR, (void*)$4)); }
		 ;


%%


int vimerror(char* line, vimui* vim, const char* s){
	ERROR("%s at token \"%s\" columns (%d - %d)\n", s, vimtext, vimlloc.first_column, vimlloc.last_column);

	vimlloc.first_column = -1;
	return 0;
}

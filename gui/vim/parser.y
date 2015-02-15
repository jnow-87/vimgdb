%define api.prefix {vim}
%locations

%{
	#include <common/log.h>
	#include <event.h>
	#include <lexer.lex.h>


	int vimerror(const char* s);
%}

%union{
	char* sptr;
	unsigned int num;

	vim_event_t event;
}


/* terminals */
%token NEWLINE
%token <sptr> STRING
%token <num> NUMBER
%token <variable> EVENT

/* non-terminals */


%%


line :	NUMBER arg-list NEWLINE											{}	/* reply */
	 |	NUMBER ':' EVENT '=' NUMBER arg-list NEWLINE					{}	/* event */
	 |	EVENT arg-list NEWLINE											{}	/* special messages */
	 ;

arg-list :	%empty														{}
		 |	arg-list ' ' NUMBER											{}
		 |	arg-list ' ' NUMBER '/' NUMBER								{}
		 |	arg-list ' ' STRING			 								{}
		 |	arg-list ' ' '!' STRING			 							{}
		 |	arg-list ' ' '"' STRING '"' 								{}
		 ;


%%


int vimerror(const char* s){
	ERROR("%s at token \"%s\" columns (%d - %d)\n", s, vimtext, vimlloc.first_column, vimlloc.last_column);

	vimlloc.first_column = -1;
	return 0;
}

%option prefix="vim"
%option noyywrap

%{
	#include <common/log.h>
	#include <common/string.h>
	#include <event.hash.h>
	#include <parser.tab.h>


	/* define YY_USER_ACTION, executed for every token, before excuting its
	 * action code
	 */
	#define YY_USER_ACTION \
		{ \
			if(vimlloc.first_column == -1) \
				vim_col = 0; \
			\
			/* update location based on current token */ \
			vimlloc.first_line = vimlloc.last_line = 0; \
			vimlloc.first_column = vim_col; \
			vimlloc.last_column = vim_col + vimleng; \
			vim_col += vimleng; \
		}
%}

/* start states */
%x QSTRING


%%


%{
	/* code that goes to the beginning of yylex() */
	static unsigned int vim_col = 0;
%}

	/* general patterns */
\r?\n									{ vim_col = 0; return NEWLINE; }
[0-9]*									{ vimlval.num = atoi(vimtext); return NUMBER; }

	/* unquoted string */
[a-zA-Z][a-zA-Z0-9_\-.]*				{
											/* check if string is a known vim variable */
											if(vim_event::lookup(vimtext, vimleng) != 0){
												vimlval.event = *vim_event::lookup(vimtext, vimleng);
												return EVENT;
											}
											else{
												vimlval.sptr = stralloc(vimtext, vimleng);
												return STRING;
											}
										}

	/* quoted string */
\"										{ BEGIN QSTRING; return vimtext[0]; }
\"\"									{ vimlval.sptr = 0; return STRING; }
<QSTRING>\"								{ BEGIN INITIAL; return vimtext[0]; }
<QSTRING>([^\"]|(\\\"))*				{ vimlval.sptr = stralloc(vimtext, vimleng); return STRING; }

	/* default */
.										{ return vimtext[0]; }

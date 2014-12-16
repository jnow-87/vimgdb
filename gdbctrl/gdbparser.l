%option prefix="gdb"
%option noyywrap

%{
	#include <gdbparser.tab.h>


	/* define YY_USER_ACTION, executed for every token, before excuting its
	 * action code
	 */
	#define YY_USER_ACTION \
		{ \
			/* update location based on current token */ \
			gdblloc.first_line = gdblloc.last_line = 0; \
			gdblloc.first_column = gdb_col; \
			gdblloc.last_column = gdb_col + gdbleng; \
			gdb_col += gdbleng; \
		}
%}

/* start states */
%x QSTRING


%%

%{
	/* code that goes to the beginning of yylex() */
	static unsigned int gdb_col = 0;
%}

"done" |
"running" |
"connected" |
"error" |
"exit"						{ return RESULT_CLASS; }
"stopped"					{ return ASYNC_CLASS; }
"(gdb)"						{ return GDB; }

\n\r?						{ return NEWLINE; }
[0-9]*						{ return NUMBER; }
[a-zA-Z][a-zA-Z0-9_\- \n]*	{ printf("lex string: \"%s\"\n", gdbtext); return STRING; }

\"							{ BEGIN QSTRING; return gdbtext[0]; }
<QSTRING>\"					{ BEGIN INITIAL; return gdbtext[0]; }
<QSTRING>([^\"]|(\\\"))*	{ printf("lex qstring: \"%s\"\n", gdbtext); return STRING; }

.							{ return gdbtext[0]; }

%%

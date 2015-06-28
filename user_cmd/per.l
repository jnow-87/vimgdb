%option prefix="per"
%option noyywrap
%option nounput

%{
	#include <common/log.h>
	#include <common/string.h>
	#include <user_cmd/pertypes.h>
	#include <user_cmd/per.tab.h>


	/* define YY_USER_ACTION, executed for every token, before excuting its
	 * action code
	 */
	#define YY_USER_ACTION \
		{ \
			/* update location based on current token */ \
			perlloc.first_line = perlloc.last_line = per_line; \
			perlloc.first_column = per_col; \
			perlloc.last_column = per_col + perleng; \
			per_col += perleng; \
		}
%}


%x COMMENT

%%


%{
	/* code that goes to the beginning of yylex() */
	static unsigned int per_col = 0;
	static unsigned int per_line = 1;
%}

	/* per identifier */
"range"					{ return RANGE; }
"register"				{ return REGISTER; }
"bits"					{ return BITS; }

	/* general patterns */
[\ \t]					{ /* ignore whitespaces */}
^[\ \t]*\r?\n			{ per_line++; /* ignore empty/white-space lines */ }
\r?\n					{ per_col = 0; per_line++; return '\n'; }
[0-9]*					{ perlval.i = atoi(pertext); return INT; }
0x[0-9]*				{ perlval.addr = (void*)strtoll(pertext, 0, 16); return ADDR; }

	/* quoted string */
\"([^\"]|(\\\"))*\"		{ perlval.sptr = stralloc(pertext + 1, perleng - 2); return STRING; }

	/* commets */
"/*"					{ BEGIN COMMENT; }
<COMMENT>\n				{ per_line++; }
<COMMENT>.				{ /* ignore */ }
<COMMENT>"*/"\r?\n?		{ BEGIN INITIAL; }
"/*"[^*/]*"*/"\r?\n		{ per_line++; }	/* multi-line comment */
^"//".*\r?\n			{ per_line++; }	/* single-line comment at beginning of line */
"//".*\r?\n				{ per_line++; }	/* single-line comment at end of line (do not read the newline */

	/* default */
<<EOF>>					{ return END; }
.						{ return pertext[0]; }
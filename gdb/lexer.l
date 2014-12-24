%option prefix="gdb"
%option noyywrap

%{
	#include <common/xmalloc.h>
	#include <gdb/result.h>
	#include <parser.tab.h>


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

	/* prototypes */
	char* alloc_string(char* s, unsigned int len);
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

"stopped" |
"breakpoint-created"		{ return ASYNC_CLASS; }

"(gdb)"						{ return GDB; }

\n\r?						{ gdb_col = 0; return NEWLINE; }
[0-9]*						{ gdblval.num = atoi(gdbtext); return NUMBER; }
[a-zA-Z][a-zA-Z0-9_\-]*		{ gdblval.sptr = alloc_string(gdbtext, gdbleng); return STRING; }

\"							{ BEGIN QSTRING; return gdbtext[0]; }
<QSTRING>\"					{ BEGIN INITIAL; return gdbtext[0]; }
<QSTRING>([^\"]|(\\\"))*	{ gdblval.sptr = alloc_string(gdbtext, gdbleng); return STRING; }

.							{ return gdbtext[0]; }

%%

extern unsigned int token;

char* alloc_string(char* _s, unsigned int len){
	char* s;
	unsigned int tk = token;

	token++;


printf("[%d] alloc string (\"%s\")\n", tk, _s);
	s = (char*)xmalloc(len + 1);
	if(s != 0)
		strncpy(s, _s, len + 1);
printf("\n");
	return s;
}

%option prefix="gdb"
%option noyywrap

%{
	#include <common/xmalloc.h>
	#include <gdb/gdb.h>
	#include <gdb/result.h>
	#include <gdb/parser.tab.h>
	#include <common/log.h>


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

	/* gdb result classes */
"done"						{ gdblval.num = RC_DONE; return RESULT_CLASS; }
"running"					{ gdblval.num = RC_RUNNING; return RESULT_CLASS; }
"connected"					{ gdblval.num = RC_CONNECTED; return RESULT_CLASS; }
"error"						{ gdblval.num = RC_ERROR; return RESULT_CLASS; }
"exit"						{ gdblval.num = RC_EXIT; return RESULT_CLASS; }

	/* gdb async output classes */
"stopped"					{ gdblval.num = AC_STOPPED; return ASYNC_CLASS; }
"breakpoint-created"		{ gdblval.num = AC_BREAK_CREATED; return ASYNC_CLASS; }
"thread-group-added"		{ gdblval.num = AC_THREAD_GRP_ADDED; return ASYNC_CLASS; }

	/* gdb EOL string */
"(gdb)"\ ?					{ return GDB; }

	/* general patterns */
\r?\n						{ gdb_col = 0; return NEWLINE; }
[0-9]*						{ gdblval.num = atoi(gdbtext); return NUMBER; }
[a-zA-Z][a-zA-Z0-9_\-.]*	{ gdblval.sptr = alloc_string(gdbtext, gdbleng); return STRING; }

	/* quoted string */
\"							{ BEGIN QSTRING; return gdbtext[0]; }
<QSTRING>\"					{ BEGIN INITIAL; return gdbtext[0]; }
<QSTRING>([^\"]|(\\\"))*	{ gdblval.sptr = alloc_string(gdbtext, gdbleng); return STRING; }

	/* MI command */
^[0-9]*-.*\n				{ gdbtext[gdbleng - 1] = 0; WARN("ignore MI command \"%s\\n\"\n", gdbtext); }

	/* default */
.							{ return gdbtext[0]; }

%%

char* alloc_string(char* _s, unsigned int len){
	char* s;

	s = (char*)xmalloc(len + 1);
	if(s != 0)
		strncpy(s, _s, len + 1);
	return s;
}

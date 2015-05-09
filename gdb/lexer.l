%option prefix="gdb"
%option noyywrap
%option nounput

%{
	#include <gdb/gdb.h>
	#include <gdb/result.h>
	#include <gdb/parser.tab.h>
	#include <gdb/identifier.hash.h>
	#include <common/log.h>
	#include <common/string.h>


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

	/* gdb result classes */
"done"									{ gdblval.num = RC_DONE; return RESULT_CLASS; }
"running"								{ gdblval.num = RC_RUNNING; return RESULT_CLASS; }
"connected"								{ gdblval.num = RC_CONNECTED; return RESULT_CLASS; }
"error"									{ gdblval.num = RC_ERROR; return RESULT_CLASS; }
"exit"									{ gdblval.num = RC_EXIT; return RESULT_CLASS; }
"stopped"								{ gdblval.num = RC_STOPPED; return RESULT_CLASS; }
"breakpoint-created"					{ gdblval.num = RC_BREAK_CREATED; return RESULT_CLASS; }
"breakpoint-modified"					{ gdblval.num = RC_BREAK_MODIFIED; return RESULT_CLASS; }
"breakpoint-deleted"					{ gdblval.num = RC_BREAK_DELETED; return RESULT_CLASS; }
"thread-created"						{ gdblval.num = RC_THREAD_CREATED; return RESULT_CLASS; }
"thread-exited"							{ gdblval.num = RC_THREAD_EXITED; return RESULT_CLASS; }
"thread-selected"						{ gdblval.num = RC_THREAD_SELECTED; return RESULT_CLASS; }
"thread-group-added"					{ gdblval.num = RC_THREAD_GRP_ADDED; return RESULT_CLASS; }
"thread-group-started"					{ gdblval.num = RC_THREAD_GRP_STARTED; return RESULT_CLASS; }
"thread-group-exited"					{ gdblval.num = RC_THREAD_GRP_EXITED; return RESULT_CLASS; }
"cmd-param-changed"						{ gdblval.num = RC_PARAM_CHANGED; return RESULT_CLASS; }
"library-loaded"						{ gdblval.num = RC_LIB_LOADED; return RESULT_CLASS; }
"library-unloaded"						{ gdblval.num = RC_LIB_UNLOADED; return RESULT_CLASS; }

	/* gdb EOL string */
"(gdb)"\ ?								{ return GDBTK; }

	/* general patterns */
\r?\n									{ gdb_col = 0; return NEWLINE; }
[0-9]*									{ gdblval.num = atoi(gdbtext); return NUMBER; }

	/* gdb identifier and unquoted string */
[a-zA-Z][a-zA-Z0-9_\-.]*				{
											/* check if string is a known gdb identifier */
											if(identifier::lookup(gdbtext, gdbleng) != 0){
												gdblval.var_id = identifier::lookup(gdbtext, gdbleng);
												return IDENTIFIER;
											}
											else{
												gdblval.sptr = stralloc(gdbtext, gdbleng);
												return STRING;
											}
										}

	/* quoted string */
\"										{ BEGIN QSTRING; return gdbtext[0]; }
<QSTRING>\"								{ BEGIN INITIAL; return gdbtext[0]; }
<QSTRING>([^\"]|(\\\"))*				{ gdblval.sptr = stralloc(gdbtext, gdbleng); return STRING; }

	/* MI command */
^[0-9]*-.*\n							{ gdbtext[gdbleng - 1] = 0; GDB("ignore MI command \"%s\\n\"\n", gdbtext); }

	/* default */
.										{ return gdbtext[0]; }

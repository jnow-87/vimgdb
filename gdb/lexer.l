%option prefix="gdb"
%option noyywrap
%option nounput

%{
	#include <gdb/gdb.h>
	#include <gdb/types.h>
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
"done"									{ return TRC_DONE; }
"running"								{ return TRC_RUNNING; }
"connected"								{ return TRC_CONNECTED; }
"error"									{ return TRC_ERROR; }
"exit"									{ return TRC_EXIT; }
"stopped"								{ return TRC_STOPPED; }
"breakpoint-created"					{ return TRC_BREAK_CREATED; }
"breakpoint-modified"					{ return TRC_BREAK_MODIFIED; }
"breakpoint-deleted"					{ return TRC_BREAK_DELETED; }
"thread-created"						{ return TRC_THREAD_CREATED; }
"thread-exited"							{ return TRC_THREAD_EXITED; }
"thread-selected"						{ return TRC_THREAD_SELECTED; }
"thread-group-added"					{ return TRC_THREAD_GRP_ADDED; }
"thread-group-started"					{ return TRC_THREAD_GRP_STARTED; }
"thread-group-exited"					{ return TRC_THREAD_GRP_EXITED; }
"cmd-param-changed"						{ return TRC_PARAM_CHANGED; }
"library-loaded"						{ return TRC_LIB_LOADED; }
"library-unloaded"						{ return TRC_LIB_UNLOADED; }

	/* gdb EOL string */
"(gdb)"\ ?								{ return GDBTK; }

	/* general patterns */
\r?\n									{ gdb_col = 0; return NEWLINE; }
[0-9]*									{ gdblval.num = atoi(gdbtext); return NUMBER; }

	/* gdb identifier and unquoted string */
"id"									{ return VAR_ID; }
"msg"									{ return VAR_MSG; }
"name"									{ return VAR_NAME; }
"bkpt"									{ return VAR_BREAKPT; }
"bkptno"								{ return VAR_BREAKPT_NUM; }
"number"								{ return VAR_NUMBER; }
"type"									{ return VAR_TYPE; }
"catch-type"							{ return VAR_CATCH_TYPE; }
"disp"									{ return VAR_DISPOSITION; }
"enabled"								{ return VAR_ENABLED; }
"addr"									{ return VAR_ADDRESS; }
"func"									{ return VAR_FUNCTION; }
"file"									{ return VAR_FILE; }
"fullname"								{ return VAR_FULLNAME; }
"line"									{ return VAR_LINE; }
"at"									{ return VAR_AT; }
"pending"								{ return VAR_PENDING; }
"evaluated-by"							{ return VAR_EVAL_BY; }
"thread"								{ return VAR_THREAD; }
"task"									{ return VAR_TASK; }
"cond"									{ return VAR_CONDITION; }
"ignore"								{ return VAR_IGNORE; }
"enable"								{ return VAR_ENABLE; }
"traceframe-usage"						{ return VAR_TRACEFRAME; }
"static-tracepoint-marker-string-id"	{ return VAR_TRACEMARKER; }
"mask"									{ return VAR_MASK; }
"pass"									{ return VAR_PASS; }
"original-location"						{ return VAR_ORIG_LOCATION; }
"times"									{ return VAR_TIMES; }
"installed"								{ return VAR_INSTALLED; }
"what"									{ return VAR_WHAT; }
"thread-group"							{ return VAR_THREAD_GROUP; }
"thread-groups"							{ return VAR_THREAD_GROUPS; }
"param"									{ return VAR_PARAM; }
"value"									{ return VAR_VALUE; }
"thread-id"								{ return VAR_THREAD_ID; }
"group-id"								{ return VAR_GROUP_ID; }
"pid"									{ return VAR_PROC_ID; }
"macro-info"							{ return VAR_MACRO_INFO; }
"target-name"							{ return VAR_TGT_NAME; }
"host-name"								{ return VAR_HOST_NAME; }
"symbols-loaded"						{ return VAR_SYM_LOADED; }
"reason"								{ return VAR_REASON; }
"frame"									{ return VAR_FRAME; }
"arg"									{ return VAR_ARG; }
"args"									{ return VAR_ARGS; }
"stopped-threads"						{ return VAR_THREADS_STOPPED; }
"core"									{ return VAR_CORE; }
"signal-name"							{ return VAR_SIG_NAME; }
"signal-meaning"						{ return VAR_SIG_MEANING; }
"gdb-result-var"						{ return VAR_GDBRES_VAR; }
"return-value"							{ return VAR_RETVAL; }
"numchild"								{ return VAR_NUM_CHILD; }
"has_more"								{ return VAR_HAS_MORE; }
"changelist"							{ return VAR_CHANGELIST; }
"in_scope"								{ return VAR_INSCOPE; }
"type_changed"							{ return VAR_TYPE_CHANGED; }
"ndeleted"								{ return VAR_NDELETED; }
"lang"									{ return VAR_LANG; }
"exp"									{ return VAR_EXP; }
"child"									{ return VAR_CHILD; }
"children"								{ return VAR_CHILDS; }
"level"									{ return VAR_LEVEL; }
"stack"									{ return VAR_STACK; }
"variables"								{ return VAR_VARIABLES; }
"exit-code"								{ return VAR_EXITCODE; }
"format"								{ return VAR_FORMAT; }
"register-names"						{ return VAR_REG_NAMES; }
"memory"								{ return VAR_MEMORY; }
"begin"									{ return VAR_BEGIN; }
"end"									{ return VAR_END; }
"offset"								{ return VAR_OFFSET; }
"contents"								{ return VAR_CONTENTS; }

[a-zA-Z][a-zA-Z0-9_\-.]*				{ gdblval.sptr = stralloc(gdbtext, gdbleng); return STRING; }

	/* quoted string */
\"										{ BEGIN QSTRING; return gdbtext[0]; }
<QSTRING>\"								{ BEGIN INITIAL; return gdbtext[0]; }
<QSTRING>([^\"]|(\\\"))*				{ gdblval.sptr = stralloc(gdbtext, gdbleng); return STRING; }

	/* MI command */
^[0-9]*-.*\n							{ gdbtext[gdbleng - 1] = 0; GDB("ignore MI command \"%s\\n\"\n", gdbtext); }

	/* default */
.										{ return gdbtext[0]; }

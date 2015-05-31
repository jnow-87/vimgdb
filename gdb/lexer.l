%option prefix="gdb"
%option noyywrap
%option nounput

%{
	#include <common/log.h>
	#include <gdb/gdb.h>
	#include <gdb/types.h>
	#include <gdb/result.h>
	#include <gdb/parser.tab.h>


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
"done"									{ gdblval.rclass = RC_DONE; return RC_TK_DONE; }
"running"								{ gdblval.rclass = RC_RUNNING; return RC_TK_RUNNING; }
"connected"								{ gdblval.rclass = RC_CONNECTED; return RC_TK_CONNECTED; }
"error"									{ gdblval.rclass = RC_ERROR; return RC_TK_ERROR; }
"exit"									{ gdblval.rclass = RC_EXIT; return RC_TK_EXIT; }
"stopped"								{ gdblval.rclass = RC_STOPPED; return RC_TK_STOPPED; }
"breakpoint-created"					{ gdblval.rclass = RC_BREAK_CREATED; return RC_TK_BREAK_CREATED; }
"breakpoint-modified"					{ gdblval.rclass = RC_BREAK_MODIFIED; return RC_TK_BREAK_MODIFIED; }
"breakpoint-deleted"					{ gdblval.rclass = RC_BREAK_DELETED; return RC_TK_BREAK_DELETED; }
"thread-created"						{ gdblval.rclass = RC_THREAD_CREATED; return RC_TK_THREAD_CREATED; }
"thread-exited"							{ gdblval.rclass = RC_THREAD_EXITED; return RC_TK_THREAD_EXITED; }
"thread-selected"						{ gdblval.rclass = RC_THREAD_SELECTED; return RC_TK_THREAD_SELECTED; }
"thread-group-added"					{ gdblval.rclass = RC_THREAD_GRP_ADDED; return RC_TK_THREAD_GRP_ADDED; }
"thread-group-started"					{ gdblval.rclass = RC_THREAD_GRP_STARTED; return RC_TK_THREAD_GRP_STARTED; }
"thread-group-exited"					{ gdblval.rclass = RC_THREAD_GRP_EXITED; return RC_TK_THREAD_GRP_EXITED; }
"cmd-param-changed"						{ gdblval.rclass = RC_PARAM_CHANGED; return RC_TK_PARAM_CHANGED; }
"library-loaded"						{ gdblval.rclass = RC_LIB_LOADED; return RC_TK_LIB_LOADED; }
"library-unloaded"						{ gdblval.rclass = RC_LIB_UNLOADED; return RC_TK_LIB_UNLOADED; }

	/* gdb identifier */
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
"new_type"								{ return VAR_NEW_TYPE; }
"new_num_children"						{ return VAR_NEW_NUM_CHILD; }
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

	/* gdb EOL string */
"(gdb)"\ ?								{ return GDBTK; }

	/* general patterns */
\r?\n									{ gdb_col = 0; return NEWLINE; }
[0-9]*									{ gdblval.num = atoi(gdbtext); return NUMBER; }

	/* unquoted string */
[a-zA-Z][a-zA-Z0-9_\-.]*				{ gdblval.string.s = gdbtext; gdblval.string.len = gdbleng; return STRING; }

	/* quoted string */
\"										{ BEGIN QSTRING; return gdbtext[0]; }
<QSTRING>\"								{ BEGIN INITIAL; return gdbtext[0]; }
<QSTRING>([^\"]|(\\\"))*				{ gdblval.string.s = gdbtext; gdblval.string.len = gdbleng; return STRING; }

	/* MI command */
^[0-9]*-.*\n							{ gdbtext[gdbleng - 1] = 0; GDB("ignore MI command \"%s\\n\"\n", gdbtext); }

	/* default */
.										{ return gdbtext[0]; }

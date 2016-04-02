%define api.prefix {gdb}
%locations

%{
	#include <common/log.h>
	#include <common/list.h>
	#include <common/string.h>
	#include <gdb/gdb.h>
	#include <gdb/types.h>
	#include <gdb/lexer.lex.h>


	/* macros */
	// extended error messages
	#define YYERROR_VERBOSE


	/* prototypes */
	int gdberror(char* line, gdbif* gdb, const char* s);
%}

%union{
	char* sptr;
	bool boolean;
	unsigned int num;
	long long int llnum;
	gdb_result_t* result;
	gdb_result_class_t rclass;
	gdb_event_t* evt;
	gdb_event_stop_t* evt_stop;
	gdb_breakpoint_t* bkpt;
	gdb_variable_t* var;
	gdb_frame_t* frame;
	gdb_location_t* loc;
	gdb_memory_t* mem;
	gdb_strlist_t* slst;

	struct{
		gdb_result_t* result;
		gdb_result_class_t rclass;
	} record;

	struct{
		char* s;
		unsigned int len;
	} string;
}


%parse-param { char* line }
%parse-param { gdbif* gdb }

%initial-action{
	gdb_scan_string(line);
}

/* terminals */
// general
%token NEWLINE
%token GDBTK

// gdb variables
%token VAR_ID
%token VAR_MSG
%token VAR_NAME
%token VAR_BREAKPT
%token VAR_BREAKPT_NUM
%token VAR_NUMBER
%token VAR_TYPE
%token VAR_CATCH_TYPE
%token VAR_DISPOSITION
%token VAR_ENABLED
%token VAR_ADDRESS
%token VAR_FUNCTION
%token VAR_FILE
%token VAR_FULLNAME
%token VAR_LINE
%token VAR_AT
%token VAR_PENDING
%token VAR_EVAL_BY
%token VAR_THREAD
%token VAR_TASK
%token VAR_CONDITION
%token VAR_IGNORE
%token VAR_ENABLE
%token VAR_TRACEFRAME
%token VAR_TRACEMARKER
%token VAR_MASK
%token VAR_PASS
%token VAR_ORIG_LOCATION
%token VAR_TIMES
%token VAR_INSTALLED
%token VAR_WHAT
%token VAR_THREAD_GROUP
%token VAR_THREAD_GROUPS
%token VAR_THREADS_STOPPED
%token VAR_PARAM
%token VAR_VALUE
%token VAR_THREAD_ID
%token VAR_GROUP_ID
%token VAR_PROC_ID
%token VAR_MACRO_INFO
%token VAR_TGT_NAME
%token VAR_HOST_NAME
%token VAR_SYM_LOADED
%token VAR_REASON
%token VAR_FRAME
%token VAR_ARG
%token VAR_ARGS
%token VAR_CORE
%token VAR_SIG_NAME
%token VAR_SIG_MEANING
%token VAR_GDBRES_VAR
%token VAR_RETVAL
%token VAR_NUM_CHILD
%token VAR_HAS_MORE
%token VAR_CHANGELIST
%token VAR_INSCOPE
%token VAR_TYPE_CHANGED
%token VAR_NDELETED
%token VAR_LANG
%token VAR_EXP
%token VAR_CHILD
%token VAR_CHILDS
%token VAR_LEVEL
%token VAR_STACK
%token VAR_VARIABLES
%token VAR_EXITCODE
%token VAR_FORMAT
%token VAR_REG_NAMES
%token VAR_MEMORY
%token VAR_BEGIN
%token VAR_END
%token VAR_OFFSET
%token VAR_CONTENTS
%token VAR_NEW_TYPE
%token VAR_NEW_NUM_CHILD

// result classes
%token <rclass> RC_TK_DONE
%token <rclass> RC_TK_RUNNING
%token <rclass> RC_TK_CONNECTED
%token <rclass> RC_TK_ERROR
%token <rclass> RC_TK_EXIT
%token <rclass> RC_TK_STOPPED
%token <rclass> RC_TK_BREAK_CREATED
%token <rclass> RC_TK_BREAK_MODIFIED
%token <rclass> RC_TK_BREAK_DELETED
%token <rclass> RC_TK_THREAD_CREATED
%token <rclass> RC_TK_THREAD_EXITED
%token <rclass> RC_TK_THREAD_SELECTED
%token <rclass> RC_TK_THREAD_GRP_ADDED
%token <rclass> RC_TK_THREAD_GRP_STARTED
%token <rclass> RC_TK_THREAD_GRP_EXITED
%token <rclass> RC_TK_PARAM_CHANGED
%token <rclass> RC_TK_LIB_LOADED
%token <rclass> RC_TK_LIB_UNLOADED

// others
%token <string> STRING
%token <num> NUMBER

/* non-terminals */
%type <sptr> string
%type <sptr> string-dummy
%type <boolean> string-bool
%type <num> string-num
%type <llnum> string-llnum-hex
%type <num> token

%type <record> response
%type <record> event
%type <rclass> response-res-class

%type <evt> event-running
%type <evt_stop> event-stop
%type <evt_stop> event-stop-body
%type <result> result
%type <slst> list
%type <slst> list-dummy
%type <slst> strlist
%type <slst> strlist-dummy
%type <bkpt> breakpoint
%type <bkpt> breakpoint-body
%type <loc> location
%type <loc> location-body
%type <var> variable
%type <var> variable-body
%type <var> children
%type <var> children-body
%type <var> variable-list
%type <var> variable-list-body
%type <frame> frame
%type <frame> frame-body
%type <frame> callstack
%type <frame> callstack-body
%type <mem> memory
%type <mem> memory-body
%type <slst> reg-names
%type <slst> value


%%


/* start */
output :	out-of-band-record GDBTK NEWLINE												{ };

/* out-of-band-record */
out-of-band-record :		%empty															{ }
				   |		out-of-band-record async-record									{ }
				   |		out-of-band-record stream-record								{ }
				   ;

/* sub-records */
async-record :				error NEWLINE													{ gdb->mi_proc_result(RC_ERROR, 0, 0); }			/* allow continued parsing after error */
			 |				token '^' response NEWLINE										{ gdb->mi_proc_result($3.rclass, $1, $3.result); }	/* result-record */
			 |				token '*' event	NEWLINE											{ gdb->mi_proc_async($3.rclass, $1, $3.result); }	/* exec-async-output */
			 |				token '+' event	NEWLINE											{ gdb->mi_proc_async($3.rclass, $1, $3.result); }	/* status-async-output */
			 |				token '=' event NEWLINE											{ gdb->mi_proc_async($3.rclass, $1, $3.result); }	/* notify-async-output */
			 ;

stream-record :				'~' string NEWLINE												{ gdb->mi_proc_stream(SC_CONSOLE, $2); }			/* console-stream-output */
			  |				'@' string NEWLINE												{ gdb->mi_proc_stream(SC_CONSOLE, $2); }			/* target-system-output */
			  |				'&' string NEWLINE												{ gdb->mi_proc_stream(SC_CONSOLE, $2); }			/* log-stream-output */
			  ;

/* response */
response :					response-res-class												{ $$.rclass = $1; $$.result = 0; }					/* result-class only */
		 |					response-res-class ',' result									{ $$.rclass = $1; $$.result = $3; }					/* normal result */
		 |					RC_TK_ERROR ',' VAR_MSG '=' strlist								{ $$.rclass = $1; $$.result = $5; }					/* error */
		 ;

response-res-class :		RC_TK_DONE														{ $$ = $1; }
				   |		RC_TK_RUNNING													{ $$ = $1; }
				   ;

result :					breakpoint														{ $$ = $1; }
	   |					location														{ $$ = $1; }
	   |					variable														{ $$ = $1; }
	   |					children														{ $$ = $1; }
	   |					variable-list													{ $$ = $1; }
	   |					frame															{ $$ = $1; }
	   |					callstack														{ $$ = $1; }
	   |					memory															{ $$ = $1; }
	   |					reg-names														{ $$ = $1; }
	   |					value															{ $$ = $1; }
	   ;

/* event */
event :						RC_TK_RUNNING ',' event-running									{ $$.rclass = $1; $$.result = $3; }
	  |						RC_TK_STOPPED ',' event-stop									{ $$.rclass = $1; $$.result = $3; }
	  |						RC_TK_CONNECTED ',' event-dummy									{ $$.rclass = $1; $$.result = 0; }
	  |						RC_TK_EXIT ',' event-dummy										{ $$.rclass = $1; $$.result = 0; }
	  |						RC_TK_BREAK_CREATED ',' event-dummy								{ $$.rclass = $1; $$.result = 0; }
	  |						RC_TK_BREAK_MODIFIED ',' event-dummy							{ $$.rclass = $1; $$.result = 0; }
	  |						RC_TK_BREAK_DELETED ',' event-dummy								{ $$.rclass = $1; $$.result = 0; }
	  |						RC_TK_THREAD_CREATED ',' event-dummy							{ $$.rclass = $1; $$.result = 0; }
	  |						RC_TK_THREAD_EXITED ',' event-dummy								{ $$.rclass = $1; $$.result = 0; }
	  |						RC_TK_THREAD_SELECTED ',' event-dummy							{ $$.rclass = $1; $$.result = 0; }
	  |						RC_TK_THREAD_GRP_ADDED event-dummy								{ $$.rclass = $1; $$.result = 0; }
	  |						RC_TK_THREAD_GRP_STARTED ',' event-dummy						{ $$.rclass = $1; $$.result = 0; }
	  |						RC_TK_THREAD_GRP_EXITED ',' event-dummy							{ $$.rclass = $1; $$.result = 0; }
	  |						RC_TK_PARAM_CHANGED ',' event-dummy								{ $$.rclass = $1; $$.result = 0; }
	  |						RC_TK_LIB_LOADED ',' event-dummy								{ $$.rclass = $1; $$.result = 0; }
	  |						RC_TK_LIB_UNLOADED ',' event-dummy								{ $$.rclass = $1; $$.result = 0; }
	  ;

event-running :				VAR_THREAD_ID '=' string-num									{ $$ = new gdb_event_t; $$->thread_id = $3; };

event-stop :				VAR_REASON '=' string											{ $$ = new gdb_event_stop_t; $$->reason = $3; }
		   |				VAR_REASON '=' string ',' event-stop-body						{ $$ = $5; $$->reason = $3; }
		   |				event-stop-body													{ $$ = $1; $$->reason = 0; }
		   ;

event-stop-body :			%empty															{ $$ = new gdb_event_stop_t; }
				|			event-stop-body con-com frame									{ $$ = $1; ((gdb_event_stop_t*)($$))->frame = $3; }
				|			event-stop-body con-com VAR_THREAD_ID '=' string-num			{ $$ = $1; $$->thread_id = $5; }
				|			event-stop-body con-com VAR_THREADS_STOPPED '=' string-dummy	{ }
				|			event-stop-body con-com VAR_CORE '=' string-dummy				{ }
				|			event-stop-body con-com VAR_DISPOSITION '=' string-dummy		{ }
				|			event-stop-body con-com VAR_BREAKPT_NUM '=' string-dummy		{ }
				|			event-stop-body con-com VAR_SIG_NAME '=' string					{ $$ = $1; $$->signal = $5; }
				|			event-stop-body con-com VAR_SIG_MEANING '=' string-dummy		{ }
				|			event-stop-body con-com VAR_GDBRES_VAR '=' string-dummy			{ }
				|			event-stop-body con-com VAR_RETVAL '=' string-dummy				{ }
				;

event-dummy :				%empty															{ }
			|				event-dummy con-com VAR_THREAD_ID '=' string-dummy				{ }
			|				event-dummy con-com VAR_THREAD_GROUP '=' string-dummy			{ }
			|				event-dummy con-com VAR_ID '=' string-dummy						{ }
			|				event-dummy con-com VAR_PARAM '=' string-dummy					{ }
			|				event-dummy con-com VAR_VALUE '=' string-dummy					{ }
			|				event-dummy con-com VAR_PROC_ID '=' string-dummy				{ }
			|				event-dummy con-com VAR_GROUP_ID '=' string-dummy				{ }
			|				event-dummy con-com VAR_TGT_NAME '=' string-dummy				{ }
			|				event-dummy con-com VAR_HOST_NAME '=' string-dummy				{ }
			|				event-dummy con-com VAR_SYM_LOADED '=' string-dummy				{ }
			|				event-dummy con-com VAR_EXITCODE '=' string-dummy				{ }
			|				event-dummy con-com breakpoint									{ delete $3; }
			;

/* gdb types */
breakpoint :				VAR_BREAKPT '=' '{' breakpoint-body '}'							{ $$ = $4; };
breakpoint-body :			%empty															{ $$ = new gdb_breakpoint_t; }
				|			breakpoint-body con-com VAR_NUMBER '=' string-num				{ $$ = $1; $$->num = $5; }
				|			breakpoint-body con-com VAR_LINE '=' string-num					{ $$ = $1; $$->line = $5; }
				|			breakpoint-body con-com VAR_FILE '=' string						{ $$ = $1; $$->filename = $5; }
				|			breakpoint-body con-com VAR_FULLNAME '=' string					{ $$ = $1; $$->fullname = $5; }
				|			breakpoint-body con-com VAR_ENABLED '=' string-bool				{ $$ = $1; $$->enabled = $5; }
				|			breakpoint-body con-com VAR_AT '=' string						{ $$ = $1; $$->at = $5; }
				|			breakpoint-body con-com VAR_CONDITION '=' string				{ $$ = $1; $$->condition = $5; }
				|			breakpoint-body con-com VAR_IGNORE '=' string					{ $$ = $1; $$->ignore_cnt = $5; }
				|			breakpoint-body con-com VAR_DISPOSITION '=' string				{ $$ = $1; $$->temporary = (strcmp($5, "del") == 0) ? true : false; delete [] $5; }
				|			breakpoint-body con-com VAR_TYPE '=' string-dummy				{ }
				|			breakpoint-body con-com VAR_ADDRESS '=' string-dummy			{ }
				|			breakpoint-body con-com VAR_FUNCTION '=' string-dummy			{ }
				|			breakpoint-body con-com VAR_ORIG_LOCATION '=' string-dummy		{ }
				|			breakpoint-body con-com VAR_TIMES '=' string-dummy				{ }
				|			breakpoint-body con-com VAR_THREAD_GROUPS '=' list-dummy		{ }
				;

location :					VAR_LINE '=' string-num ',' location-body						{ $$ = $5; $5->line = $3; };
location-body :				%empty															{ $$ = new gdb_location_t; }
			  |				location-body con-com VAR_FILE '=' string						{ $$ = $1; $$->filename = $5; }
			  |				location-body con-com VAR_FULLNAME '=' string					{ $$ = $1; $$->fullname = $5; }
			  |				location-body con-com VAR_MACRO_INFO '=' string-dummy			{ }
			  ;

variable :					VAR_NAME '=' string ',' variable-body							{ $$ = $5; $5->name = $3; }
		 |					VAR_LANG '=' string-dummy ',' variable-body						{ $$ = $5; }
		 |					VAR_FORMAT '=' string-dummy ',' variable-body					{ $$ = $5; }
		 |					VAR_NDELETED '=' string-dummy									{ $$ = 0; }
		 ;

variable-body :				%empty															{ $$ = gdb_variable_t::acquire(); }
			  |				variable-body con-com VAR_TYPE '=' string						{ $$ = $1; $$->type = $5; }
			  |				variable-body con-com VAR_VALUE '=' string						{ $$ = $1; $$->value = strdeescape($5); }
			  |				variable-body con-com VAR_NUM_CHILD '=' string-num				{ $$ = $1; $$->nchilds = $5; }
			  |				variable-body con-com VAR_EXP '=' string						{ $$ = $1; $$->exp = $5; }
			  |				variable-body con-com VAR_ARG '=' string-dummy					{ $$ = $1; $$->argument = true; }
			  |				variable-body con-com VAR_INSCOPE '=' string-bool				{ $$ = $1; $$->inscope = $5; }
			  |				variable-body con-com VAR_TYPE_CHANGED '=' string-bool			{ $$ = $1; $$->type_changed = $5; }
			  |				variable-body con-com VAR_NEW_TYPE '=' string					{ $$ = $1; $$->type = $5; }
			  |				variable-body con-com VAR_NEW_NUM_CHILD '=' string-num			{ $$ = $1; $$->nchilds = $5; }
			  |				variable-body con-com VAR_HAS_MORE '=' string-dummy				{ }
			  |				variable-body con-com VAR_LANG '=' string-dummy					{ }
			  |				variable-body con-com VAR_THREAD_ID '=' string-dummy			{ }
			  ;

variable-list :				VAR_CHANGELIST '=' '[' variable-list-body ']'					{ $$ = $4; }
			 |				VAR_VARIABLES '=' '[' variable-list-body ']'					{ $$ = $4; }
			 ;

variable-list-body :		%empty															{ $$ = 0; }
				  |			variable-list-body con-com '{' variable '}'						{ $$ = $1; list_add_tail(&$$, $4); }
				  ;

children :					VAR_NUM_CHILD '=' string-dummy ',' children-body				{ $$ = $5; };
children-body :				%empty															{ $$ = 0; }
			  |				VAR_CHILDS '=' '[' children-body ']'							{ $$ = $4; }
			  |				children-body con-com VAR_CHILD '=' '{' variable '}'			{ $$ = $1; list_add_tail(&$$, $6); }
			  |				children-body con-com VAR_HAS_MORE '=' string-dummy				{ }
			  ;


callstack :					VAR_STACK '=' '[' callstack-body ']'							{ $$ = $4; };
callstack-body :			%empty															{ $$ = 0; }
				|			callstack-body con-com frame									{ $$ = $1; list_add_head(&$$, $3); }
				;

frame :						VAR_FRAME '=' '{' frame-body '}'								{ $$ = $4; };
frame-body :				%empty															{ $$ = gdb_frame_t::acquire(); }
		   |				frame-body con-com VAR_ADDRESS '=' string-llnum-hex				{ $$ = $1; $$->addr = (void*)$5; }
		   |				frame-body con-com VAR_LINE '=' string-num						{ $$ = $1; $$->line = $5; }
		   |				frame-body con-com VAR_LEVEL '=' string-num						{ $$ = $1; $$->level = $5; }
		   |				frame-body con-com VAR_FUNCTION '=' string						{ $$ = $1; $$->function = $5; }
		   |				frame-body con-com VAR_FILE '=' string							{ $$ = $1; $$->filename = $5; }
		   |				frame-body con-com VAR_FULLNAME '=' string						{ $$ = $1; $$->fullname = $5; }
		   |				frame-body con-com VAR_ARGS '=' '[' arg-list ']'				{ }
		   ;

arg-list :					%empty															{ }
		 |					arg-list con-com '{' arg-list '}'								{ }
		 |					arg-list con-com VAR_NAME '=' string-dummy						{ }
		 |					arg-list con-com VAR_VALUE '=' string-dummy						{ }
		 ;

memory :					VAR_MEMORY '=' '[' '{' memory-body '}' ']'						{ $$ = $5; };
memory-body :				%empty															{ $$ = gdb_memory_t::acquire(); }
			|				memory-body con-com VAR_BEGIN '=' string						{ $$ = $1; $$->begin = $5; }
			|				memory-body con-com VAR_END '=' string-llnum-hex				{ $$ = $1; $$->length = (unsigned int)($5 - strtoll($$->begin, 0, 16)); }
			|				memory-body con-com VAR_CONTENTS '=' string						{ $$ = $1; $$->content = $5; }
			|				memory-body con-com VAR_OFFSET '=' string-dummy					{ }
			;

reg-names :					VAR_REG_NAMES '=' list											{ $$ = $3; };

value :						VAR_VALUE '=' strlist											{ $$ = $3; strdeescape($3->s); };

/* common */
list :						'[' strlist ']'													{ $$ = $2; };
list-dummy :				'[' strlist-dummy ']'											{ };

strlist :					%empty															{ $$ = 0; }
	 	|					strlist con-com string											{ $$ = $1; list_add_tail(&$$, new gdb_strlist_t($3)); }
		;

strlist-dummy :				%empty															{ }
			  |				strlist-dummy con-com string-dummy								{ }
			  ;

string :					'"' '"'															{ $$ = stralloc((char*)"", 1); }
	   |					'"' STRING '"'													{ $$ = stralloc($2.s, $2.len); }
	   ;

string-bool :				'"' '"'															{ $$ = false; }
			|				'"' STRING '"'													{ $$ = ($2.s[0] == 't' || $2.s[0] == 'y') ? true : false; }
			;

string-num :				'"' '"'															{ $$ = 0; }
		   |				'"' STRING '"'													{ $$ = atoi($2.s); }
		   ;

string-llnum-hex :			'"' '"'															{ $$ = 0; }
			 	 |			'"' STRING '"'													{ $$ = strtoll($2.s, 0, 16); }
				 ;

string-dummy :				'"' '"'															{ }
			 |				'"' STRING '"'													{ }
			 ;

token :						%empty															{ $$ = 0; }
	  |						NUMBER															{ $$ = $1; }
	  ;

con-com :					%empty															{ }
	    |					','																{ }
	    ;


%%


int gdberror(char* line, gdbif* gdb, const char* s){
	USER("gdbparse: %s at token \"%s\" columns (%d - %d)\nline: \"%s\"\n\n", s, gdbtext, gdblloc.first_column, gdblloc.last_column, line);
	return 0;
}

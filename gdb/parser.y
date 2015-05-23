%define api.prefix {gdb}
%locations

%{
	#include <common/log.h>
	#include <common/list.h>
	#include <common/string.h>
	#include <gdb/gdb.h>
	#include <gdb/types.h>
	#include <gdb/identifier.h>
	#include <gdb/lexer.lex.h>


	/* macros */
	// extended error messages
	#define YYERROR_VERBOSE


	/* prototypes */
	int gdberror(char* line, gdbif* gdb, const char* s);
%}

%union{
	char* sptr;
	unsigned int num;

	void* result;
	gdb_result_class_t rclass;
	const gdb_id_t* var_id;

	struct{
		void* result;
	} record;

	gdb_strlist_t* slst;
	gdb_breakpoint_t* breakpt;
	gdb_location_t* loc;
	gdb_variable_t* var;
	gdb_frame_t* frame;
	gdb_memory_t* mem;
	gdb_event_t* evt;
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

// result classes
%token TRC_DONE
%token TRC_RUNNING
%token TRC_CONNECTED
%token TRC_ERROR
%token TRC_EXIT
%token TRC_STOPPED
%token TRC_BREAK_CREATED
%token TRC_BREAK_MODIFIED
%token TRC_BREAK_DELETED
%token TRC_THREAD_CREATED
%token TRC_THREAD_EXITED
%token TRC_THREAD_SELECTED
%token TRC_THREAD_GRP_ADDED
%token TRC_THREAD_GRP_STARTED
%token TRC_THREAD_GRP_EXITED
%token TRC_PARAM_CHANGED
%token TRC_LIB_LOADED
%token TRC_LIB_UNLOADED


%token <sptr> STRING
%token <num> NUMBER
%token <var_id> IDENTIFIER

/* non-terminals */
%type <record> record
%type <result> result
%type <rclass> record-res-class
%type <slst> list
%type <slst> list-dummy
%type <slst> strlist
%type <slst> strlist-dummy
%type <sptr> string
%type <sptr> string-dummy
%type <num> token
%type <breakpt> breakpoint
%type <breakpt> breakpoint-body
%type <loc> location
%type <loc> location-body
%type <var> variable
%type <var> variable-body
%type <var> children
%type <var> children-body
%type <var> variablelist
%type <var> variablelist-body
%type <frame> frame
%type <frame> frame-body
%type <frame> frame-list
%type <frame> frame-list-body
%type <mem> memory
%type <mem> memory-body
%type <slst> reg-names
%type <sptr> value
%type <evt> event
%type <evt> event-running
%type <evt> event-stop
%type <evt> event-stop-body


%%


output :	out-of-band-record GDBTK NEWLINE												{ }
	   ;


/* out-of-band-record */
out-of-band-record :		%empty															{ }
				   |		out-of-band-record async-record									{ }
				   |		out-of-band-record stream-record								{ }
				   ;

async-record :				error NEWLINE													{ /*gdb->mi_proc_result(TRC_ERROR, 0, (gdb_result_t*)0);*/ }				/* allow continued parsing after error */
			 |				token '*' event	NEWLINE											{ /*gdb->mi_proc_async($3.rclass, $1, (gdb_result_t*)$3.result);*/ }	/* exec-async-output */
			 |				token '+' event	NEWLINE											{ /*gdb->mi_proc_async($3.rclass, $1, (gdb_result_t*)$3.result);*/ }	/* status-async-output */
			 |				token '=' event NEWLINE											{ /*gdb->mi_proc_async($3.rclass, $1, (gdb_result_t*)$3.result);*/ }	/* notify-async-output */
			 |				token '^' record NEWLINE										{ /*gdb->mi_proc_result($3.rclass, $1, (gdb_result_t*)$3.result);*/ }	/* result-record */
			 ;

record :					record-res-class												{ /*$$.rclass = $1; $$.result = 0;*/ }						/* result-class only */
	   |					record-res-class ',' result										{ /*$$.rclass = $1; $$.result = $3;*/ }						/* normal result */
	   |					TRC_ERROR ',' VAR_MSG '=' string								{ /*$$.rclass = $1; $$.result = (gdb_result_t*)$5;*/ }		/* error */
	   ;

record-res-class :			TRC_DONE														{ }
				 |			TRC_RUNNING														{ }
				 ;

stream-record :				'~' string NEWLINE												{ gdb->mi_proc_stream(SC_CONSOLE, $2); }	/* console-stream-output */
			  |				'@' string NEWLINE												{ gdb->mi_proc_stream(SC_CONSOLE, $2); }	/* target-system-output */
			  |				'&' string NEWLINE												{ gdb->mi_proc_stream(SC_CONSOLE, $2); }	/* log-stream-output */
			  ;

result :					breakpoint														{ $$ = (void*)$1; }
	   |					location														{ $$ = (void*)$1; }
	   |					variable														{ $$ = (void*)$1; }
	   |					children														{ $$ = (void*)$1; }
	   |					variablelist													{ $$ = (void*)$1; }
	   |					frame															{ $$ = (void*)$1; }
	   |					frame-list														{ $$ = (void*)$1; }
	   |					memory															{ $$ = (void*)$1; }
	   |					reg-names														{ $$ = (void*)$1; }
	   |					value															{ $$ = (void*)$1; }
	   ;

event :						TRC_RUNNING ',' event-running									{ $$ = $3; }
	  |						TRC_STOPPED ',' event-stop										{ $$ = $3; }
	  |						TRC_CONNECTED ',' event-dummy									{ }
	  |						TRC_EXIT ',' event-dummy										{ }
	  |						TRC_BREAK_CREATED ',' event-dummy								{ }
	  |						TRC_BREAK_MODIFIED ',' event-dummy								{ }
	  |						TRC_BREAK_DELETED ',' event-dummy								{ }
	  |						TRC_THREAD_CREATED ',' event-dummy								{ }
	  |						TRC_THREAD_EXITED ',' event-dummy								{ }
	  |						TRC_THREAD_SELECTED ',' event-dummy								{ }
	  |						TRC_THREAD_GRP_ADDED event-dummy								{ }
	  |						TRC_THREAD_GRP_STARTED ',' event-dummy							{ }
	  |						TRC_THREAD_GRP_EXITED ',' event-dummy							{ }
	  |						TRC_PARAM_CHANGED ',' event-dummy								{ }
	  |						TRC_LIB_LOADED ',' event-dummy									{ }
	  |						TRC_LIB_UNLOADED ',' event-dummy								{ }
	  ;

/* gdb events */
event-running :				VAR_THREAD_ID '=' string										{ printf("alloc event\n"); $$ = new gdb_event_t; $$->thread_id = atoi($3); delete $3; };

event-stop :				VAR_REASON '=' string											{ printf("alloc stop event\n"); $$ = new gdb_event_stop_t; $$->reason = $3; }
		   |				VAR_REASON '=' string ',' event-stop-body						{ $$ = $5; $$->reason = $3; }
		   ;

event-stop-body :			%empty															{ printf("alloc stop event\n"); $$ = new gdb_event_stop_t; }
				|			event-stop-body con-com frame									{ $$ = $1; ((gdb_event_stop_t*)($$))->frame = $3; }
				|			event-stop-body con-com VAR_THREAD_ID '=' string				{ $$ = $1; $$->thread_id = atoi($5); delete $5; }
				|			event-stop-body con-com VAR_THREADS_STOPPED '=' string-dummy	{ }
				|			event-stop-body con-com VAR_CORE '=' string-dummy				{ }
				|			event-stop-body con-com VAR_DISPOSITION '=' string-dummy		{ }
				|			event-stop-body con-com VAR_BREAKPT_NUM '=' string-dummy		{ }
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
			|				event-dummy con-com breakpoint									{ printf("del bkpt\n"); delete $3; }
			;


/* gdb types */
breakpoint :				VAR_BREAKPT '=' '{' breakpoint-body '}'							{ $$ = $4; };
breakpoint-body :			%empty															{ printf("alloc brkpt\n"); $$ = new gdb_breakpoint_t; }
				|			breakpoint-body con-com VAR_NUMBER '=' string					{ $$ = $1; $$->num = atoi($5); delete $5; }
				|			breakpoint-body con-com VAR_LINE '=' string						{ $$ = $1; $$->line = atoi($5); delete $5; }
				|			breakpoint-body con-com VAR_FILE '=' string						{ $$ = $1; $$->filename = $5; }
				|			breakpoint-body con-com VAR_FULLNAME '=' string					{ $$ = $1; $$->fullname = $5; }
				|			breakpoint-body con-com VAR_ENABLED '=' string					{ $$ = $1; $$->enabled = (strcmp((const char*)$5, "y") == 0) ? true : false; delete $5; }
				|			breakpoint-body con-com VAR_AT '=' string						{ $$ = $1; $$->at = $5; }
				|			breakpoint-body con-com VAR_CONDITION '=' string				{ $$ = $1; $$->condition = $5; }
				|			breakpoint-body con-com VAR_IGNORE '=' string					{ $$ = $1; $$->ignore_cnt = $5; }
				|			breakpoint-body con-com VAR_DISPOSITION '=' string				{ $$ = $1; $$->temporary = (strcmp($5, "del") == 0) ? true : false; delete $5; }
				|			breakpoint-body con-com VAR_TYPE '=' string-dummy				{ }
				|			breakpoint-body con-com VAR_ADDRESS '=' string-dummy			{ }
				|			breakpoint-body con-com VAR_FUNCTION '=' string-dummy			{ }
				|			breakpoint-body con-com VAR_ORIG_LOCATION '=' string-dummy		{ }
				|			breakpoint-body con-com VAR_TIMES '=' string-dummy				{ }
				|			breakpoint-body con-com VAR_THREAD_GROUPS '=' list-dummy		{ }
				;

location :					VAR_LINE '=' string ',' location-body							{ $$ = $5; $5->line = atoi($3); delete $3; };
location-body :				%empty															{ printf("alloc location\n"); $$ = new gdb_location_t; }
			  |				location-body con-com VAR_FILE '=' string						{ $$ = $1; $$->filename = $5; }
			  |				location-body con-com VAR_FULLNAME '=' string					{ $$ = $1; $$->fullname = $5; }
			  |				location-body con-com VAR_MACRO_INFO '=' string-dummy			{ }
			  ;

variable :					VAR_NAME '=' string ',' variable-body							{ $$ = $5; $5->name = $3; }
		 |					VAR_LANG '=' string-dummy ',' variable-body						{ $$ = $5; }
		 |					VAR_FORMAT '=' string-dummy ',' variable-body					{ $$ = $5; }
		 ;

variable-body :				%empty															{ printf("alloc var\n"); $$ = gdb_variable_t::acquire(); }
			  |				variable-body con-com VAR_TYPE '=' string						{ $$ = $1; $$->type = $5; }
			  |				variable-body con-com VAR_VALUE '=' string						{ $$ = $1; $$->value = strdeescape($5); }
			  |				variable-body con-com VAR_NUM_CHILD '=' string					{ $$ = $1; $$->nchilds = atoi($5); delete $5; }
			  |				variable-body con-com VAR_EXP '=' string						{ $$ = $1; $$->exp = $5; }
			  |				variable-body con-com VAR_ARG '=' string-dummy					{ $$ = $1; $$->argument = true; }
			  |				variable-body con-com VAR_INSCOPE '=' string					{ $$ = $1; $$->inscope = $5[0]; }
			  |				variable-body con-com VAR_HAS_MORE '=' string-dummy				{ }
			  |				variable-body con-com VAR_LANG '=' string-dummy					{ }
			  |				variable-body con-com VAR_THREAD_ID '=' string-dummy			{ }
			  |				variable-body con-com VAR_TYPE_CHANGED '=' string-dummy			{ }
			  ;

children :					VAR_NUM_CHILD '=' string-dummy ',' children-body				{ $$ = $5; };
children-body :				%empty															{ printf("start children-list\n"); $$ = 0; }
			  |				VAR_CHILDS '=' '[' children-body ']'							{ $$ = $4; }
			  |				children-body con-com VAR_CHILD '=' '{' variable '}'			{ $$ = $1; list_add_tail(&$$, $6); }
			  |				children-body con-com VAR_HAS_MORE '=' string-dummy				{ }
			  ;

variablelist :				VAR_CHANGELIST '=' '[' variablelist-body ']'					{ $$ = $4; }
			 |				VAR_VARIABLES '=' '[' variablelist-body ']'						{ $$ = $4; }
			 ;

variablelist-body :			%empty															{ printf("start var-list\n"); $$ = 0; }
				|			variablelist-body con-com '{' variable '}'						{ $$ = $1; list_add_tail(&$$, $4); }
				;

frame-list :				VAR_STACK '=' '[' frame-list-body ']'							{ $$ = $4; };
frame-list-body :			%empty															{ printf("start frame-list\n"); $$ = 0; }
				|			frame-list-body con-com frame									{ $$ = $1; list_add_tail(&$$, $3); }
				;

frame :						VAR_FRAME '=' '{' frame-body '}'								{ $$ = $4; };
frame-body :				%empty															{ printf("alloc frame\n"); $$ = new gdb_frame_t; }
		   |				frame-body con-com VAR_ADDRESS '=' string						{ $$ = $1; $$->addr = (void*)strtoll($5, 0, 16); delete $5; }
		   |				frame-body con-com VAR_LINE '=' string							{ $$ = $1; $$->line = atoi($5); delete $5; }
		   |				frame-body con-com VAR_LEVEL '=' string							{ $$ = $1; $$->level = atoi($5); delete $5; }
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
memory-body :				%empty															{ printf("alloc memory\n"); $$ = new gdb_memory_t; }
			|				memory-body con-com VAR_BEGIN '=' string						{ $$ = $1; $$->begin = $5; }
			|				memory-body con-com VAR_END '=' string							{ $$ = $1; $$->length = (unsigned int)(strtoll($5, 0, 16) - strtoll($$->begin, 0, 16)); delete $5; }
			|				memory-body con-com VAR_CONTENTS '=' string						{ $$ = $1; $$->content = $5; }
			|				memory-body con-com VAR_OFFSET '=' string-dummy					{ }
			;

reg-names :					VAR_REG_NAMES '=' list											{ $$ = $3; }
		  ;

value :						VAR_VALUE '=' string											{ $$ = $3; }
	  ;

/* common */
list :						'[' strlist ']'													{ $$ = $2; }
	 ;

list-dummy :				'[' strlist-dummy ']'											{ $$ = $2; }
		   ;

strlist :					%empty															{ $$ = 0; }
	 	|					strlist con-com string											{ $$ = $1; list_add_tail(&$1, new gdb_strlist_t($3)); }
		;

strlist-dummy :				%empty															{ }
			  |				strlist-dummy con-com string-dummy								{ }
			  ;

string :					'"' '"'															{ $$ = stralloc((char*)"", 1); }
	   |					'"' STRING '"'													{ $$ = $2; };
	   ;

string-dummy :				'"' '"'															{ }
			 |				'"' STRING '"'													{ delete $2; }
			 ;

token :						%empty															{ $$ = 0; }
	  |						NUMBER															{ $$ = $1; printf("token %d\n", $1);}
	  ;

con-com :					%empty															{ }
	    |					','																{ }
	    ;


%%


int gdberror(char* line, gdbif* gdb, const char* s){
	USER("gdbparse: %s at token \"%s\" columns (%d - %d)\n", s, gdbtext, gdblloc.first_column, gdblloc.last_column);
	return 0;
}

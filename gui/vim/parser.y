%define api.prefix {vim}
%locations

%{
	#include <common/log.h>
	#include <common/string.h>
	#include <gui/vim/vimui.h>
	#include <gui/vim/cursor.h>
	#include <lexer.lex.h>


	/* macros */
	// extended error messages
	#define YYERROR_VERBOSE


	/* prototypes */
	int vimerror(char* line, vimui* vim, const char* s);


	/* static variables */
	static int ret_val;
%}

%union{
	char dummy;
	int num;
	bool boolean;

	vim_event_id_t event_id;

	vim_event_t* event;
	vim_reply_t* reply;
	vim_cursor_t* cursor;

	struct{
		char* s;
		unsigned int len;
	} string;
}

%parse-param { char* line }
%parse-param { vimui* vim }

%initial-action{
	vim_scan_string(line);
	ret_val = 0;
}


/* terminals */
%token NEWLINE
%token DISCONNECT
%token STARTUPDONE
%token KILLED
%token FILEOPENED
%token INSERT
%token REMOVE
%token KEYCOMMAND
%token KEYATPOS
%token NEWDOTANDMARK
%token VERSION
%token <event_id> SPECIAL
%token <string> STRING
%token <num> NUMBER
%token <boolean> BOOL

/* non-terminals */
%type <reply> reply
%type <cursor> cursor
%type <event> event
%type <event> special
%type <string> string
%type <dummy> list-dummy



%%

start :			line																	{ return ret_val; }

line :			%empty																	{ }
	 |			line NUMBER reply NEWLINE												{ ret_val |= vim->proc_reply($2, $3); }		/* reply */
	 |			line NUMBER ':' event NEWLINE											{ ret_val |= vim->proc_event($2, $4); }		/* event */
	 |			line special NEWLINE													{ ret_val |= vim->proc_event(0, $2); }		/* special event */
	 ;

/* reply */
reply :			%empty																	{ $$ = 0; }
	  |			cursor																	{ $$ = $1; }
	  ;


cursor :		' ' NUMBER ' ' NUMBER ' ' NUMBER ' ' NUMBER								{ $$ = new vim_cursor_t; $$->bufid = $2; $$->line = $4;	$$->column = $6; };

/* event */
event :			%empty																	{ $$ = new vim_event_t; }	// event is not actually recursive, but eases allocation
	  |			event DISCONNECT '=' NUMBER												{ $$ = $1; $$->evt_id = E_DISCONNECT; }
	  |			event STARTUPDONE '=' NUMBER											{ $$ = $1; $$->evt_id = E_STARTUPDONE; }
	  |			event KILLED '=' NUMBER													{ $$ = $1; $$->evt_id = E_KILLED; }
	  |			event FILEOPENED '=' NUMBER ' ' string ' ' BOOL ' ' BOOL				{ $$ = $1; $$->evt_id = E_FILEOPENED; $$->data = stralloc($6.s, $6.len); }
	  |			event INSERT '=' NUMBER ' ' NUMBER ' ' string							{ $$ = $1; $$->evt_id = E_INSERT; }
	  |			event REMOVE '=' NUMBER ' ' NUMBER ' ' NUMBER							{ $$ = $1; $$->evt_id = E_REMOVE; }
	  |			event KEYCOMMAND '=' NUMBER ' ' string									{ $$ = $1; $$->evt_id = E_KEYCOMMAND; $$->data = stralloc($6.s, $6.len); }
	  |			event KEYATPOS '=' NUMBER ' ' string ' ' NUMBER ' ' NUMBER '/' NUMBER	{ $$ = $1; $$->evt_id = E_KEYATPOS; }
	  |			event VERSION '=' NUMBER ' ' '"' string '"'								{ $$ = $1; $$->evt_id = E_VERSION; }
	  |			event NEWDOTANDMARK '=' NUMBER ' ' NUMBER ' ' NUMBER					{ $$ = $1; $$->evt_id = E_NEWDOTANDMARK; }
	  ;

/* special event */
special :	SPECIAL list-dummy															{ $$ = new vim_event_t; $$->evt_id = $1; };

/* common */
list-dummy :	%empty																	{ }
		   |	list-dummy ' ' BOOL														{ }
		   |	list-dummy ' ' '!' STRING												{ }
		   |	list-dummy ' ' string													{ }
		   |	list-dummy ' ' NUMBER													{ }
		   |	list-dummy ' ' NUMBER '/' NUMBER										{ }
		   ;

string :		STRING																	{ $$ = $1; }
	   |		'"' STRING '"'															{ $$ = $2; }
	   ;


%%


int vimerror(char* line, vimui* vim, const char* s){
	ERROR("%s at token \"%s\" columns (%d - %d)\n", s, vimtext, vimlloc.first_column, vimlloc.last_column);

	vimlloc.first_column = -1;
	return 0;
}

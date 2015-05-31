%define api.prefix {vim}
%locations

%{
	#include <common/log.h>
	#include <gui/vim/vimui.h>
	#include <gui/vim/cursor.h>
	#include <gui/vim/length.h>
	#include <lexer.lex.h>


	/* macros */
	// extended error messages
	#define YYERROR_VERBOSE


	/* prototypes */
	int vimerror(char* line, vimui* vim, const char* s);
%}

%union{
	char* sptr;
	char dummy;
	int num;
	bool boolean;

	vim_event_id_t event_id;

	vim_event_t* event;
	vim_reply_t* reply;
	vim_length_t* length;
	vim_cursor_t* cursor;
}

%parse-param { char* line }
%parse-param { vimui* vim }

%initial-action{
	vim_scan_string(line);
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
%token <sptr> STRING
%token <num> NUMBER
%token <boolean> BOOL

/* non-terminals */
%type <reply> reply
%type <length> length
%type <cursor> cursor
%type <event> event
%type <event> special
%type <sptr> string
%type <dummy> list-dummy



%%


line :			NUMBER reply NEWLINE													{ return vim->proc_reply($1, $2); }		/* reply */
	 |			NUMBER ':' event NEWLINE												{ return vim->proc_event($1, $3); }		/* event */
	 |			special NEWLINE															{ return vim->proc_event(0, $1); }		/* special event */
	 ;

/* reply */
reply :			%empty																	{ $$ = 0; }
	  |			length																	{ $$ = $1; }
	  |			cursor																	{ $$ = $1; }
	  ;


length :		' ' NUMBER																{ $$ = new vim_length_t; $$->value = $2; };
cursor :		' ' NUMBER ' ' NUMBER ' ' NUMBER ' ' NUMBER								{ $$ = new vim_cursor_t; $$->bufid = $2; $$->line = $4;	$$->column = $6; };

/* event */
event :			%empty																	{ $$ = new vim_event_t; }	// event is not actually recursive, but eases allocation
	  |			event DISCONNECT '=' NUMBER												{ $$ = $1; $$->evt_id = E_DISCONNECT; }
	  |			event STARTUPDONE '=' NUMBER											{ $$ = $1; $$->evt_id = E_STARTUPDONE; }
	  |			event KILLED '=' NUMBER													{ $$ = $1; $$->evt_id = E_KILLED; }
	  |			event FILEOPENED '=' NUMBER ' ' string ' ' BOOL ' ' BOOL				{ $$ = $1; $$->evt_id = E_FILEOPENED; $$->data = $6; }
	  |			event INSERT '=' NUMBER ' ' NUMBER ' ' string							{ $$ = $1; $$->evt_id = E_INSERT; delete $8; }
	  |			event REMOVE '=' NUMBER ' ' NUMBER ' ' NUMBER							{ $$ = $1; $$->evt_id = E_REMOVE; }
	  |			event KEYCOMMAND '=' NUMBER ' ' string									{ $$ = $1; $$->evt_id = E_KEYCOMMAND; $$->data = $6; }
	  |			event KEYATPOS '=' NUMBER ' ' string ' ' NUMBER ' ' NUMBER '/' NUMBER	{ $$ = $1; $$->evt_id = E_KEYATPOS; delete $6; }
	  |			event VERSION '=' NUMBER ' ' '"' string '"'								{ $$ = $1; $$->evt_id = E_VERSION; delete $7; }
	  |			event NEWDOTANDMARK '=' NUMBER ' ' NUMBER ' ' NUMBER					{ $$ = $1; $$->evt_id = E_NEWDOTANDMARK; }
	  ;

/* special event */
special :	SPECIAL list-dummy															{ $$ = new vim_event_t; $$->evt_id = $1; };

/* common */
list-dummy :	%empty																	{ }
		   |	list-dummy ' ' BOOL														{ }
		   |	list-dummy ' ' '!' STRING												{ delete $4; }
		   |	list-dummy ' ' string													{ delete $3; }
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

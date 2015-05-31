%option prefix="vim"
%option noyywrap
%option nounput

%{
	#include <common/log.h>
	#include <gui/vim/vimui.h>
	#include <gui/vim/event.h>
	#include <gui/vim/cursor.h>
	#include <parser.tab.h>


	/* define YY_USER_ACTION, executed for every token, before excuting its
	 * action code
	 */
	#define YY_USER_ACTION \
		{ \
			if(vimlloc.first_column == -1) \
				vim_col = 0; \
			\
			/* update location based on current token */ \
			vimlloc.first_line = vimlloc.last_line = 0; \
			vimlloc.first_column = vim_col; \
			vimlloc.last_column = vim_col + vimleng; \
			vim_col += vimleng; \
		}
%}

/* start states */
%x QSTRING


%%


%{
	/* code that goes to the beginning of yylex() */
	static unsigned int vim_col = 0;
%}

	/* general patterns */
\r?\n									{ vim_col = 0; return NEWLINE; }
-?[0-9]*								{ vimlval.num = atoi(vimtext); return NUMBER; }
T										{ vimlval.boolean = true; return BOOL; }
F										{ vimlval.boolean = false; return BOOL; }

	/* events */
"disconnect"							{ return DISCONNECT; }
"startupDone"							{ return STARTUPDONE; }
"killed"								{ return KILLED; }
"fileOpened"							{ return FILEOPENED; }
"insert"								{ return INSERT; }
"remove"								{ return REMOVE; }
"keyCommand"							{ return KEYCOMMAND; }
"keyAtPos"								{ return KEYATPOS; }
"version"								{ return VERSION; }
"newDotAndMark"							{ return NEWDOTANDMARK; }

	/* special events */
"DISCONNECT"							{ vimlval.event_id = E_DISCONNECT; return SPECIAL; }
"ACCEPT"								{ vimlval.event_id = E_ACCEPT; return SPECIAL; }
"AUTH"									{ vimlval.event_id = E_AUTH; return SPECIAL; }
"DETACH"								{ vimlval.event_id = E_DETACH; return SPECIAL; }

	/* unquoted string */
[a-zA-Z][a-zA-Z0-9_\-.]*				{ vimlval.string.s = vimtext; vimlval.string.len = vimleng; return STRING; }

	/* quoted string */
\"										{ BEGIN QSTRING; return vimtext[0]; }
\"\"									{ vimlval.string.s = 0; vimlval.string.len = 0; return STRING; }
<QSTRING>\"								{ BEGIN INITIAL; return vimtext[0]; }
<QSTRING>([^\"]|(\\\"))*				{ vimlval.string.s = vimtext; vimlval.string.len = vimleng; return STRING; }

	/* default */
.										{ return vimtext[0]; }

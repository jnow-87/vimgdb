%define api.prefix {gdb}
%locations

%{
	#include <stdio.h>
	#include <gdbparser.lex.h>


	int gdberror(const char* s);
%}

%union{
	char* sptr;
	unsigned int num;
}

%token NEWLINE
%token GDB
%token ASYNC_CLASS
%token RESULT_CLASS
%token <sptr> STRING
%token <num> NUMBER


%%


output :	out-of-band-record result-record GDB NEWLINE			{ printf("output\n"); };

/* out-of-band-record */
out-of-band-record :	%empty										{ printf("oob-empty\n"); }
				   |	out-of-band-record async-record				{ printf("oob-async\n"); }
				   |	out-of-band-record stream-record			{ printf("oob-stream\n"); }
				   ;

async-record :			exec-async-output							{}
			 |			status-async-output							{}
			 |			notify-async-output							{}
			 ;

exec-async-output :		token '*' async-output NEWLINE				{};
status-async-output :	token '+' async-output NEWLINE				{};
notify-async-output :	token '=' async-output NEWLINE				{};

async-output :			ASYNC_CLASS									{}
			 |			ASYNC_CLASS ',' result-list					{}
			 ;

stream-record :			console-stream-output						{}
			  |			targte-system-output						{}
			  |			log-stream-output							{printf("stream\n");}
			  ;

console-stream-output :	'~' '"' STRING '"' NEWLINE					{};
targte-system-output :	'@' '"' STRING '"' NEWLINE					{};
log-stream-output :		'&' '"' STRING '"' NEWLINE					{printf("log-stream\n");};


/* result-record */
result-record :		%empty											{}
			  |		token '^' RESULT_CLASS NEWLINE					{}
			  |		token '^' RESULT_CLASS ',' result-list NEWLINE	{}
			  ;


/* common */
result :			variable '=' value								{};
result-list :		result											{}
		    |		result-list ',' result							{}
	   		;

value :				const											{}
	  |				tuple											{}
	  |				list											{}
	  ;

value-list :		value											{}
		   |		value-list ',' value							{}
		   ;

const :				'"' STRING '"'									{};
tuple :				'{' '}'											{}
	  |				'{' result-list '}'								{}
	  ;

list :				'[' ']'											{}
	 |				'[' result-list ']'								{}
	 |				'[' value-list ']'								{}
	 ;

variable :			STRING											{};

token :				%empty											{}
	  |				NUMBER											{}
	  ;


%%


#ifdef TEST


char* line;


int main(int argc, char** argv){
//	char str[] = "&\"target gdbctrl\n\"\n(gdb)\n";
	char str[] = "&\"target gdbctrl\\n\"\n&\"Undefined target command: \\\"gdbctrl\\\".  Try \\\"help target\\\".\n\n\"\n^error,msg=\"Undefined target command: \\\"gdbctrl\\\".  Try \\\"help target\\\".\"\n(gdb)\n";
//	char str[] = "&\".target gdbctrl\n\"\n&\"Undefined target command: \\\"gdbctrl\\\".  Try help target.\n\"\n^error,msg=\"Undefined target command: gdbctrl.  Try help target.\"\n(gdb)\n";


	if(argc < 2)
		line = str;
	else
		line = argv[1];

	gdb_scan_string(line);
	gdbparse();
	return 0;
}

#endif // TEST

int gdberror(const char* s){
	char tmp;
	unsigned int i;


	printf("%s at token \"%s\" columns (%d - %d)\n", s, gdbtext, gdblloc.first_column, gdblloc.last_column);

#ifdef TEST
	tmp = line[gdblloc.last_column];
	line[gdblloc.last_column] = 0;
	printf("%s\n", line);
	for(i=1; i<gdblloc.last_column; i++)
		printf(" ");
	printf("^\n");
#endif // TEST
}

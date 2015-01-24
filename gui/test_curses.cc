#include <gui/curses.h>
#include <common/tty.h>
#include <string.h>
#include <unistd.h>


#define PROMPT	"~$ "


int main(int argc, char** argv){
	char c, line[255];
	unsigned int i;
	gui* ui;
	tty std_term;


	ui = new curses();
	ui->init();

	ui->win_log("enter \"quit\" to exit\n");
	ui->win_cmd("insert commands here\n" PROMPT);

	i = 0;
	while(1){
		std_term.read(&c, 1);

		switch(c){
		case '\n':
		case '\r':
			line[i] = 0;
			i = 0;

			ui->win_log("exec: \"%s\"\n", line);
			ui->win_cmd("\n" PROMPT);

			if(strcmp(line, "quit") == 0 || strcmp(line, "q") == 0){
				ui->win_log("exit\n");
				sleep(1);
				goto end;
			}

			break;
		
		default:
			ui->win_cmd("%c", c);
			ui->win_gdb_log("%c %d\n", c, (int)c);
			line[i++] = c;
			break;
		};
	}

end:
	ui->destroy();
	delete (curses*)ui;

	return 0;
}

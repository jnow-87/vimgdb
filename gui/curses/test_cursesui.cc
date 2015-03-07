#include <gui/curses/cursesui.h>
#include <common/tty.h>
#include <string.h>
#include <unistd.h>


#define PROMPT	"~$ "


int main(int argc, char** argv){
	char c, line[255];
	unsigned int i;
	tty std_term;


	ui = new cursesui();
	ui->init();

	ui->print(WIN_APPLOG, "enter \"quit\" to exit\n");
	ui->print(WIN_CMD, "insert commands here\n" PROMPT);

	i = 0;
	while(1){
		std_term.read(&c, 1);

		switch(c){
		case '\n':
		case '\r':
			line[i] = 0;
			i = 0;

			ui->print(WIN_APPLOG, "exec: \"%s\"\n", line);
			ui->print(WIN_CMD, "\n" PROMPT);

			if(strcmp(line, "quit") == 0 || strcmp(line, "q") == 0){
				ui->print(WIN_APPLOG, "exit\n");
				sleep(1);
				goto end;
			}

			break;

		case 127:
			if(i <= 0)
				break;

			line[--i] = 0;

			ui->clearline(WIN_CMD);
			ui->print(WIN_CMD, PROMPT "%s", line);
			break;
		
		default:
			ui->print(WIN_GDBLOG, "%c %d\n", c, (int)c);
			ui->print(WIN_CMD, "%c", c);
			line[i++] = c;
			break;
		};
	}

end:
	ui->destroy();
	delete (cursesui*)ui;

	return 0;
}

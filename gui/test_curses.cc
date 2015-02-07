#include <gui/curses.h>
#include <common/tty.h>
#include <string.h>
#include <unistd.h>


#define PROMPT	"~$ "


int main(int argc, char** argv){
	char c, line[255];
	unsigned int i;
	tty std_term;


	ui = new curses();
	ui->init();

	ui->log_print("enter \"quit\" to exit\n");
	ui->cmd_print("insert commands here\n" PROMPT);

	i = 0;
	while(1){
		std_term.read(&c, 1);

		switch(c){
		case '\n':
		case '\r':
			line[i] = 0;
			i = 0;

			ui->log_print("exec: \"%s\"\n", line);
			ui->cmd_print("\n" PROMPT);

			if(strcmp(line, "quit") == 0 || strcmp(line, "q") == 0){
				ui->log_print("exit\n");
				sleep(1);
				goto end;
			}

			break;

		case 127:
			if(i <= 0)
				break;

			line[--i] = 0;

			ui->cmd_clrline();
			ui->cmd_print(PROMPT "%s", line);
			break;
		
		default:
			ui->gdblog_print("%c %d\n", c, (int)c);
			ui->cmd_print("%c", c);
			line[i++] = c;
			break;
		};
	}

end:
	ui->destroy();
	delete (curses*)ui;

	return 0;
}

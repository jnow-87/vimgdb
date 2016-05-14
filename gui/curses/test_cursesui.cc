#include <gui/curses/cursesui.h>
#include <common/tty.h>
#include <string.h>
#include <unistd.h>


#define PROMPT	"~$ "


int main(int argc, char **argv){
	char *line;
	int win_user;


	ui = new cursesui();
	ui->init(0);

	win_user = ui->win_create("user-log", true, 0);

	if(win_user < 0)
		goto end;

	ui->win_print(win_user, "enter \"quit\" to exit\n");

	while(1){
		line = ui->readline();

		if(line == 0)
			break;

		ui->win_print(win_user, "exec: \"%s\"\n", line);

		if(strcmp(line, "quit") == 0 || strcmp(line, "q") == 0){
			ui->win_print(win_user, "exit\n");
			sleep(1);
			break;
		}
	}

end:
	ui->destroy();
	delete (cursesui*)ui;

	return 0;
}

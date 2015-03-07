#include <gui/curses/cursesui.h>
#include <common/tty.h>
#include <string.h>
#include <unistd.h>


#define PROMPT	"~$ "


int main(int argc, char** argv){
	char* line;


	ui = new cursesui();
	ui->init();

	ui->print(WIN_USERLOG, "enter \"quit\" to exit\n");

	while(1){
		line = ui->readline();

		if(line == 0)
			break;

		ui->print(WIN_USERLOG, "exec: \"%s\"\n", line);

		if(strcmp(line, "quit") == 0 || strcmp(line, "q") == 0){
			ui->print(WIN_USERLOG, "exit\n");
			sleep(1);
			break;
		}
	}

	ui->destroy();
	delete (cursesui*)ui;

	return 0;
}

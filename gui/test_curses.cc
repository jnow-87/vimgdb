#include <gui/curses.h>


int main(int argc, char** argv){
	gui* g;


	g = new curses();
	g->init();

	g->win_log("press 'q' to quit");
	g->win_cmd("cmd example");

	while(getchar() != 'q');

	g->destroy();
	delete (curses*)g;

	return 0;
}

#include <gui/curses.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>


curses::curses(){
	/* init ncurses */
	setlocale(LC_ALL, "");

	initscr();
	noecho();

	/* init class members */
	nwin = 0;
	max_win = 2;
	windows = (window_t**)malloc(sizeof(window_t*) * max_win);
	memset(windows, 0x0, sizeof(window_t*) * max_win);
}

curses::~curses(){
	unsigned int i;


	/* free allocated windows */
	for(i=0; i<max_win; i++){
		if(windows[i] == 0)
			continue;

		delwin(windows[i]->win);
		free(windows[i]);
	}

	free(windows);

	/* stop ncurses */
	endwin();
}

int curses::win_create(const char* title){
	int i, id;


	/* realloc if now enough space to store windows */
	if(nwin + 1 > max_win){
		windows = (window_t**)realloc(windows, sizeof(window_t*) * max_win * 2);
		memset(windows + max_win, 0x0, sizeof(window_t*) * max_win);

		max_win *= 2;
	}

	/* search free id */
	id = -1;
	for(i=0; i<max_win; i++){
		if(windows[i] == 0){
			id = i;
			break;
		}
	}

	if(id == -1)
		return -1;

	/* allocated curses window */
	windows[id] = new window_t;
	windows[id]->win = newwin(2, 2, 0, 0);	// arbitrary width and height, final
											// values are set in win_resize()
	windows[id]->title = (char*)title;
	scrollok(windows[id]->win, true);		// enable auto-scroll
	wmove(windows[id]->win, 1, 0);			// move cursor

	nwin++;

	/* update size of all windows */
	if(win_resize() < 0){
		win_destroy(id);
		return -1;
	}

	return id;
}

int curses::win_destroy(int win_id){
	if(win_id >= max_win || windows[win_id] == 0)
		return -1;

	/* de-init window and free memory */
	delwin(windows[win_id]->win);
	free(windows[win_id]);
	windows[win_id] = 0;
	nwin--;

	/* update screen */
	win_resize();

	return 0;
}

void curses::win_write(int win_id, const char* fmt, ...){
	va_list lst;


	va_start(lst, fmt);
	win_write(win_id, fmt, lst);
	va_end(lst);
}

void curses::win_vwrite(int win_id, const char* fmt, va_list lst){
	int x, y;


	/* print string */
	vwprintw(windows[win_id]->win, fmt, lst);
	
	/* restore window title that might be overwritten due to scroll */
	getyx(windows[win_id]->win, y, x);
	mvwprintw(windows[win_id]->win, 0, 0, "  [ %s ]", windows[win_id]->title);
	wmove(windows[win_id]->win, y, x);

	/* refresh */
	wrefresh(windows[win_id]->win);
}


int curses::win_resize(){
	unsigned int i, line, col, x, y, width, height, ncols, nlines;


	/* determine number of windows in the same line (ncols)
	 * and the resulting number of lines
	 */
	ncols = COLS / min_win_width;
	if(nwin < ncols)
		ncols = nwin;

	nlines = nwin / ncols + ((nwin % ncols) ? 1 : 0);

	/* determine width and height */
	width = COLS / ncols;
	height = LINES / nlines;

	/* check if sufficient space available */
	if(width < min_win_width || height < min_win_height)
		return -1;

	/* clear screen */
	erase();
	refresh();

	/* place windows */
	line = 0;
	col = 0;
	for(i=0; i<max_win; i++){
		if(windows[i] == 0)
			continue;

		// save current cursor position
		getyx(windows[i]->win, y, x);

		// resize and move window
		wresize(windows[i]->win, height - 1, width);
		mvwin(windows[i]->win, line * height, col * width);
		mvwprintw(windows[i]->win, 0, 0, "  [ %s ]", windows[i]->title);

		// restore cursor position
		wmove(windows[i]->win, y, x);

		// make changes take effect
		wrefresh(windows[i]->win);

		// increment position
		if(++col >= ncols){
			col = 0;
			line++;
		}
	}

	return 0;
}

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
		delwin(windows[i]->frame);
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
	windows[id]->win = newwin(2, 2, 0, 0);		// arbitrary width and height, final
	windows[id]->frame = newwin(2, 2, 0, 0);	// values are set in win_resize()
	windows[id]->title = (char*)title;
	scrollok(windows[id]->win, true);			// enable auto-scroll

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
	delwin(windows[win_id]->frame);
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
	win_vwrite(win_id, fmt, lst);
	va_end(lst);
}

void curses::win_vwrite(int win_id, const char* fmt, va_list lst){
	vwprintw(windows[win_id]->win, fmt, lst);
	wrefresh(windows[win_id]->win);
}

int curses::win_resize(){
	unsigned int i, line, col, width, height, ncols, nlines;


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

		// update window frame
		wresize(windows[i]->frame, height, width);
		mvwin(windows[i]->frame, line * height, col * width);
		box(windows[i]->frame, 0, 0);
		mvwprintw(windows[i]->frame, 0, 2, "[ %s ]", windows[i]->title);
		wrefresh(windows[i]->frame);

		// update window text area
		wresize(windows[i]->win, height - 2, width - 2);
		mvwin(windows[i]->win, line * height + 1, col * width + 1);
		wrefresh(windows[i]->win);

		// increment position
		if(++col >= ncols){
			col = 0;
			line++;
		}
	}

	return 0;
}

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

/**
 * \brief	create new curses window
 *
 * \param	title		window title
 * \param	oneline		if true, the window width is maximised
 *
 * \return	>0			window id
 * 			<0			error
 */
int curses::win_create(const char* title, bool oneline){
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
	windows[id]->oneline = oneline;
	scrollok(windows[id]->win, true);			// enable auto-scroll

	nwin++;

	/* update size of all windows */
	if(win_resize() < 0){
		win_destroy(id);
		return -1;
	}

	return id + 1;
}

int curses::win_destroy(int win_id){
	win_id--;

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
	win_id--;

	vwprintw(windows[win_id]->win, fmt, lst);
	wrefresh(windows[win_id]->win);
}

/**
 * \brief	resize and rearange windows
 *
 * \return	0	success
 *			<0	error
 */
int curses::win_resize(){
	unsigned int i, j, width, height, win;
	unsigned int ncols[nwin + 1], split[nwin + 1], nsplit, nlines, max_cols;


	if(nwin == 0)
		return 0;

	memset(split, 0x0, sizeof(unsigned int) * (nwin + 1));
	memset(ncols, 0x0, sizeof(unsigned int) * (nwin + 1));

	/* split windows in lines with normal windows and
	 * horizontally maximised windows
	 *
	 * 	split	number of windows before the next 'oneline' window
	 * 	nsplit	entries in split
	 */
	nsplit = 0;
	for(i=0; i<max_win; i++){
		if(windows[i] == 0)
			continue;

		/* check if window is horizontally maximised */
		if(windows[i]->oneline){
			if(split[nsplit] != 0)
				nsplit++;
			split[nsplit++] = 1;
		}
		else
			split[nsplit]++;
	}

	if(split[nsplit] > 0)
		nsplit++;

	/* identify number of windows per line
	 *
	 * 	ncols[i]	number of windows for line i
	 * 	nlines		entries in ncols
	 */
	max_cols = COLS / min_win_width;

	nlines = 0;
	for(i=0; i<nsplit; i++){
		if(split[i] <= max_cols){
			ncols[nlines++] = split[i];
		}
		else{
			for(j=0; j<split[i] / max_cols; j++)
				ncols[nlines++] = max_cols;

			if(split[i] % max_cols)
				ncols[nlines++] = split[i] % max_cols;
		}
	}

	if(ncols[nlines] > 0)
		nlines++;

	/* update windows */
	height = LINES / nlines;
	if(height < min_win_height)
		return -1;

	// clear screen
	erase();
	refresh();

	win = 0;
	for(i=0; i<nlines; i++){
		width = COLS / ncols[i];
		if(width < min_win_width)
			return -1;

		for(j=0; j<ncols[i]; j++){
			for(;win<max_win; win++){
				if(windows[win] != 0)
					break;
			}

			// update window frame
			wresize(windows[win]->frame, height, width);
			mvwin(windows[win]->frame, i * height, j * width);
			box(windows[win]->frame, 0, 0);
			mvwprintw(windows[win]->frame, 0, 2, "[ %s ]", windows[win]->title);
			wrefresh(windows[win]->frame);

			// update window text area
			wresize(windows[win]->win, height - 2, width - 2);
			mvwin(windows[win]->win, i * height + 1, j * width + 1);
			wrefresh(windows[win]->win);

			win++;
		}
	}

	return 0;
}

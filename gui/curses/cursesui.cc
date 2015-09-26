#include <gui/curses/cursesui.h>
#include <locale.h>
#include <stdlib.h>
#include <string.h>


#define CMD_PROMPT		"cmd: "
#define MIN_WIN_HEIGHT	3
#define MIN_WIN_WIDTH	30


cursesui::cursesui(){
	/* init ncurses */
	setlocale(LC_ALL, "");

	initscr();
	noecho();

	/* init class members */
	nwin = 0;
	max_win = 2;

	pthread_mutex_init(&mutex, 0);
}

cursesui::~cursesui(){
	pthread_mutex_destroy(&mutex);

	/* stop ncurses */
	endwin();
}

int cursesui::init(){
	windows = (window_t**)malloc(sizeof(window_t*) * max_win);

	if(windows == 0)
		goto err_0;

	memset(windows, 0x0, sizeof(window_t*) * max_win);

	line_len = 256;
	line = (char*)malloc(line_len);

	if(line == 0)
		goto err_1;

	term = new tty;

	if(term == 0)
		goto err_2;

	user_win_id = win_create("command-line", true, 3);

	if(user_win_id < 0)
		goto err_3;

	return 0;

err_3:
	delete term;

err_2:
	free(line);

err_1:
	free(windows);

err_0:
	return -1;
}

void cursesui::destroy(){
	unsigned int i;


	win_destroy(user_win_id);

	/* free allocated windows */
	for(i=0; i<max_win; i++){
		if(windows[i] == 0)
			continue;

		delwin(windows[i]->win);
		delwin(windows[i]->frame);
		delete windows[i];
	}

	free(windows);
	free(line);

	delete term;
}

/**
 * \brief	read user input from gui
 *
 * \return	!= 0	pointer to line
 * 			0		error
 */
char* cursesui::readline(){
	char c;
	unsigned int i;


	if(line == 0 || term == 0)
		return 0;

	i = 0;
	win_print(user_win_id, CMD_PROMPT);

	while(1){
		term->read(&c, 1);

		if(c == '\n' || c == '\r'){
			line[i] = 0;

			win_print(user_win_id, "\n");
			return line;
		}
		else if(c == 127){
			if(i <= 0)
				continue;

			line[--i] = 0;
			win_clrline(user_win_id);
			win_print(user_win_id, CMD_PROMPT "%s", line);
		}
		else{
			win_print(user_win_id, "%c", c);
			line[i++] = c;

			if(i >= line_len){
				line_len *= 2;
				line = (char*)realloc(line, line_len);

				if(line == 0)
					return 0;
			}
		}
	}
}

int cursesui::win_atomic(int win, bool en){
	return 0;
}

/**
 * \brief	create new cursesui window
 *
 * \param	name		window title
 * \param	oneline		if true, the window width is maximised
 *
 * \return	>=0			window id
 * 			<0			error
 */
int cursesui::win_create(const char* name, bool oneline, unsigned int height){
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
		if(windows[i] == 0)
			id = i;
		else if(strcmp(windows[i]->title, name) == 0)
			return i;
	}

	if(id == -1)
		return -1;

	/* allocated cursesui window */
	windows[id] = new window_t;
	windows[id]->win = newwin(2, 2, 0, 0);		// arbitrary width and height, final
	windows[id]->frame = newwin(2, 2, 0, 0);	// values are set in win_resize()
	windows[id]->title = (char*)name;
	windows[id]->oneline = (height > 0) ? true : oneline;
	windows[id]->height = height;
	scrollok(windows[id]->win, true);			// enable auto-scroll

	nwin++;

	/* update size of all windows
	 * 	it seems to be necessary for call it twice
	 * 	to retain the content of existing windows
	 */
	if(win_resize() < 0 || win_resize() < 0){
		win_destroy(id);
		return -1;
	}

	return id;
}

int cursesui::win_destroy(int win_id){
	if(win_id >= max_win || windows[win_id] == 0)
		return -1;

	/* de-init window and free memory */
	delwin(windows[win_id]->win);
	delwin(windows[win_id]->frame);
	delete windows[win_id];
	windows[win_id] = 0;
	nwin--;

	/* update screen */
	win_resize();

	return 0;
}

int cursesui::win_getid(const char* name){
	return win_create(name);
}

int cursesui::win_anno_add(int win, int line, const char* sign, const char* color_fg, const char* color_bg){
	return -1;
}

int cursesui::win_anno_delete(int win, int line, const char* sign){
	return -1;
}

int cursesui::win_cursor_set(int win, int line, int col){
	return -1;
}

int cursesui::win_cursor_preserve(int win, bool pc){
	return -1;
}

int cursesui::win_readonly(int win, bool ro){
	return -1;
}

void cursesui::win_print(int win_id, const char* fmt, ...){
	va_list lst;


	va_start(lst, fmt);
	win_vprint(win_id, fmt, lst);
	va_end(lst);
}

void cursesui::win_vprint(int win_id, const char* fmt, va_list lst){
	pthread_mutex_lock(&mutex);

	vwprintw(windows[win_id]->win, fmt, lst);
	wrefresh(windows[win_id]->win);

	pthread_mutex_unlock(&mutex);
}

void cursesui::win_clear(int win_id){
	pthread_mutex_lock(&mutex);

	wclear(windows[win_id]->win);
	wrefresh(windows[win_id]->win);

	pthread_mutex_unlock(&mutex);
}

void cursesui::win_clrline(int win_id){
	int x, y;


	pthread_mutex_lock(&mutex);

	getyx(windows[win_id]->win, y, x);
	wmove(windows[win_id]->win, y, 0);
	wclrtoeol(windows[win_id]->win);
	wrefresh(windows[win_id]->win);

	pthread_mutex_unlock(&mutex);
}

/**
 * \brief	resize and rearange windows
 *
 * \return	0	success
 *			<0	error
 */
int cursesui::win_resize(){
	unsigned int i, j, width, height, fixed_height, nfixed_height, win, line, ncols[nwin + 1], split[nwin + 1], nsplit, nlines, max_cols;


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
	fixed_height = 0;
	nfixed_height = 0;
	for(i=0; i<max_win; i++){
		if(windows[i] == 0)
			continue;

		// check if window is horizontally maximised
		if(windows[i]->oneline){
			if(split[nsplit] != 0)
				nsplit++;
			split[nsplit++] = 1;
		}
		else
			split[nsplit]++;

		// accumulate height for windows that specify it
		// (default is 0)
		if(windows[i]->height > 0){
			fixed_height += windows[i]->height;
			nfixed_height++;
		}
	}

	if(split[nsplit] > 0)
		nsplit++;

	/* identify number of windows per line
	 *
	 * 	ncols[i]	number of windows for line i
	 * 	nlines		entries in ncols
	 */
	max_cols = COLS / MIN_WIN_WIDTH;

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
	height = 0;
	if(nlines - nfixed_height > 0){
		height = (LINES - fixed_height) / (nlines - nfixed_height);

		if(height < MIN_WIN_HEIGHT)
			return -1;
	}

	if((LINES - fixed_height) < 0)
		return -1;

	// clear screen
	erase();
	refresh();

	win = 0;
	line = 0;
	for(i=0; i<nlines; i++){
		width = COLS / ncols[i];
		if(width < MIN_WIN_WIDTH)
			return -1;

		for(j=0; j<ncols[i]; j++){
			for(;win<max_win; win++){
				if(windows[win] != 0)
					break;
			}

			// update window frame
			wresize(windows[win]->frame, (windows[win]->height > 0) ? windows[win]->height : height, width);
			mvwin(windows[win]->frame, line, j * width);
			box(windows[win]->frame, 0, 0);
			mvwprintw(windows[win]->frame, 0, 2, "[ %s ]", windows[win]->title);
			wrefresh(windows[win]->frame);

			// update window text area
			wresize(windows[win]->win, ((windows[win]->height > 0) ? windows[win]->height : height) - 2, width - 2);
			mvwin(windows[win]->win, line + 1, j * width + 1);
			wrefresh(windows[win]->win);

			win++;
		}

		line += (windows[win - 1]->height > 0) ? windows[win - 1]->height : height;
	}

	return 0;
}

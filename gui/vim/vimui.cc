#include <common/string.h>
#include <common/log.h>
#include <common/list.h>
#include <common/opt.h>
#include <common/map.h>
#include <gui/vim/vimui.h>
#include <gui/vim/event.h>
#include <gui/vim/result.h>
#include <gui/vim/cursor.h>
#include <gui/vim/length.h>
#include <cmd.hash.h>
#include <parser.tab.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>


/* class implementation */
vimui::vimui(){
	pthread_mutexattr_t attr;


	read_tid = 0;
	nbserver = 0;
	nbclient = 0;
	ostr = 0;
	cwd = opt.vim_cwd;
	cursor_update = true;

	memset((void*)&resp, -1, sizeof(response_t));

	event_lst = 0;
	list_init(event_lst);

	pthread_mutex_init(&buf_mtx, 0);
	pthread_mutex_init(&resp_mtx, 0);
	pthread_mutex_init(&event_mtx, 0);
	pthread_cond_init(&resp_avail, 0);
	pthread_cond_init(&event_avail, 0);

	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&ui_mtx, &attr);
	pthread_mutexattr_destroy(&attr);
}

vimui::~vimui(){
	pthread_cond_destroy(&resp_avail);
	pthread_cond_destroy(&event_avail);
	pthread_mutex_destroy(&resp_mtx);
	pthread_mutex_destroy(&event_mtx);
	pthread_mutex_destroy(&buf_mtx);
	pthread_mutex_destroy(&ui_mtx);
}

int vimui::init(){
	char* s;


	ostr_len = 255;
	ostr = new char[ostr_len];

	if(ostr == 0)
		goto err_0;

	nbserver = new socket(VIM_NB_PORT, 0);

	if(nbserver == 0)
		goto err_1;

	if(nbserver->init_server(TCP) != 0){
		goto err_2;
	}

	nbclient = nbserver->await_client();

	if(nbclient == 0)
		goto err_2;

	nbclient->set_timeout(0);

	if(pthread_create(&read_tid, 0, readline_thread, this) != 0)
		goto err_3;

	thread_name[read_tid] = "vim-readline";

	// wait for vim to finish init
	while(1){
		s = readline();

		if(s == 0)
			return -1;
		
		if(strcmp(s, "init-done") == 0)
			break;
	}

	return 0;

err_3:
	delete nbclient;

err_2:
	delete nbserver;

err_1:
	delete ostr;

err_0:
	return -1;
}

void vimui::destroy(){
	response_t* e;


	/* trigger shutdown of main thread */
	event(0, 0, E_DISCONNECT, 0);

	/* cancel readline-thread */
	if(read_tid != 0){
		pthread_cancel(read_tid);
		pthread_join(read_tid, 0);
	}

	/* close sockets */
	delete nbclient;
	delete nbserver;

	/* free memory */
	list_for_each(event_lst, e)
		list_rm(&event_lst, e);

	delete ostr;
}

char* vimui::readline(){
	char* line = 0;
	unsigned int len = 0;
	response_t* e;


	while(1){
		/* wait for data being read by readline-thread */
		pthread_mutex_lock(&event_mtx);

		if(list_empty(event_lst))
			pthread_cond_wait(&event_avail, &event_mtx);

		e = list_first(event_lst);
		list_rm(&event_lst, e);

		pthread_mutex_unlock(&event_mtx);

		/* process command */
		switch(e->evt_id){
		case E_KEYATPOS:
			VIM("handle KEYATPOS event\n");

			// KEYATPOS events are user commands
			// hence signal availability of a command to readline()
			/* copy response string into line */
			if(len < strlen(e->result->sptr)){
				len += strlen(e->result->sptr) + 1;
				line = new char[len];

				if(line == 0)
					goto end;
			}

			strcpy(line, e->result->sptr);
			goto end;
		
		case E_FILEOPENED:
			VIM("handle FILEOPENED event\n");

			win_create(e->result->sptr);
			break;

		case E_KILLED:
			VIM("handle KILLED event\n");

			win_destroy(e->buf_id);
			break;

		case E_DISCONNECT:
			VIM("handle DISCONNECT event\n");

			line = 0;
			goto end;

		default:
			TODO("unhandled event id %d\n", e->evt_id);
		};

		vim_result_free(e->result);
		delete e;
	}

end:
	vim_result_free(e->result);
	delete e;

	return line;
}

int vimui::atomic(bool en){
	return atomic(en, true);
}

int vimui::win_create(const char* name, bool oneline, unsigned int height){
	static int volatile bufid = 1;	// avoid using buf-id 0 for netbeans
	int id;
	buffer_t* b;


	if(name == 0)
		return -1;

	/* check if buffer for name already exists
	 * 	if so return its id, otherwise
	 * 	create new buffer
	 */
	id = win_getid(name);

	if(id > 0)
		return id;

	pthread_mutex_lock(&ui_mtx);

	id = bufid++;

	if(id < 0){
		pthread_mutex_unlock(&ui_mtx);
		return -1;
	}

	/* connect to vim buffer
	 * 	if name is absolute path assume a file to edit
	 * 	otherwise assume a non-file buffer, e.g. 'breakpoints'
	 */
	if(name[0] == '/')
		action(CMD, "editFile", id, 0, 0, "\"%s\"", name);
	else
		action(CMD, "putBufferNumber", id, 0, 0, "\"%s/%s\"", cwd, name);

	action(CMD, "stopDocumentListen", id, 0, 0, "");

	/* set buf_id as used */
	b = new buffer_t;
	b->id = id;
	b->len = 0;

	if(name[0] == '/'){
		b->name = new char[strlen(name) + 1];
		strcpy(b->name, name);
	}
	else{
		b->name = new char[strlen(name) + strlen(cwd) + 2];
		sprintf(b->name, "%s/%s", cwd, name);
	}


	pthread_mutex_lock(&buf_mtx);

	bufname_map[b->name] = b;
	bufid_map[id] = b;

	pthread_mutex_unlock(&buf_mtx);

	pthread_mutex_unlock(&ui_mtx);

	return id;
}

int vimui::win_getid(const char* name){
	static char* volatile s = 0;
	static unsigned int volatile slen = 0;
	unsigned int len;
	map<string, buffer_t*>::iterator it;


	pthread_mutex_lock(&buf_mtx);

	if(name[0] != '/'){
		len = strlen(name) + strlen(cwd) + 2;

		if(slen < len){
			delete s;
			s = new char[len];
			slen = len;
		}

		sprintf(s, "%s/%s", cwd, name);
		it = bufname_map.find(s);
	}
	else
		it = bufname_map.find(name);

	pthread_mutex_unlock(&buf_mtx);

	if(it != bufname_map.end())
		return it->second->id;
	return -1;
}

int vimui::win_destroy(int win){
	map<string, buffer_t*>::iterator it_name;
	map<int, buffer_t*>::iterator it_id;


	pthread_mutex_lock(&ui_mtx);

	/* close vim buffer */
	if(action(CMD, "close", win, 0, 0, "") != 0)
		goto err_0;

	/* remove buffer */
	pthread_mutex_lock(&buf_mtx);

	it_id = bufid_map.find(win);

	if(it_id == bufid_map.end())
		goto err_1;

	it_name = bufname_map.find(it_id->second->name);

	delete it_id->second->name;
	delete it_id->second;

	bufname_map.erase(it_name);
	bufid_map.erase(it_id);

	pthread_mutex_unlock(&buf_mtx);
	pthread_mutex_unlock(&ui_mtx);

	return 0;

err_1:
	pthread_mutex_unlock(&buf_mtx);

err_0:
	pthread_mutex_unlock(&ui_mtx);

	return -1;
}

int vimui::win_anno_add(int win, int line, const char* sign, const char* color_fg, const char* color_bg){
	static unsigned int volatile id = 1;
	buffer_t* buf;
	string key;
	int annotype;


	key = sign;
	key += color_fg;
	key += color_bg;

	pthread_mutex_lock(&ui_mtx);

	buf = MAP_LOOKUP_SAFE(bufid_map, win, buf_mtx);

	if(buf == 0)
		goto err;

	annotype = MAP_LOOKUP(buf->anno_types, key);

	atomic(true, false);

	/* create annotation type if it doesn't exist */
	if(annotype == 0){
		if(action(CMD, "defineAnnoType", win, 0, 0, "0 \"%s\" \"\" \"%s\" %s %s", key.c_str(), sign, color_fg, color_bg) != 0)
			goto err;

		buf->anno_types[key] = buf->anno_types.size();	// first element has value 1, since anno_types size
														// is incremented by using the []-operator
		annotype = MAP_LOOKUP(buf->anno_types, key);
	}

	/* add annotation */
	if(action(CMD, "addAnno", win, 0, 0, "%d %d %d/0", id, annotype, line) == 0){
		key = line;
		key += sign;

		buf->annos[key] = id;
		id++;

		atomic(false, false);

		pthread_mutex_unlock(&ui_mtx);
		return 0;
	}

err:
	atomic(false, false);
	pthread_mutex_unlock(&ui_mtx);
	return -1;
}

int vimui::win_anno_delete(int win, int line, const char* sign){
	buffer_t* buf;
	string key;
	map<string, int>::iterator anno;


	pthread_mutex_lock(&ui_mtx);

	buf = MAP_LOOKUP_SAFE(bufid_map, win, buf_mtx);

	if(buf == 0)
		goto err;

	key = line;
	key += sign;
	anno = buf->annos.find(key);

	if(anno != buf->annos.end()){
		if(action(CMD, "removeAnno", win, 0, 0, "%d", anno->second) != 0)
			goto err;

		buf->annos.erase(anno);
	}

	pthread_mutex_unlock(&ui_mtx);
	return 0;

err:
	pthread_mutex_unlock(&ui_mtx);
	return -1;
}

int vimui::win_cursor_set(int win, int line){
	int r;
	buffer_t* buf;

	pthread_mutex_lock(&ui_mtx);

	if(line == -1){
		buf = MAP_LOOKUP_SAFE(bufid_map, win, buf_mtx);

		if(buf != 0)	r = action(CMD, "setDot", win, 0, 0, "%d", buf->len - 1);
		else			r = -1;
	}
	else
		r = action(CMD, "setDot", win, 0, 0, "%d/0", line);

	pthread_mutex_unlock(&ui_mtx);

	return r;
}

void vimui::win_print(int win, const char* fmt, ...){
	va_list lst;


	if(win <= 0)
		return;

	va_start(lst, fmt);
	win_vprint(win, fmt, lst);
	va_end(lst);
}

void vimui::win_vprint(int win, const char* fmt, va_list lst){
	unsigned int len;
	va_list tlst;
	buffer_t* buf;


	buf = MAP_LOOKUP_SAFE(bufid_map, win, buf_mtx);

	if(buf == 0)
		return;

	pthread_mutex_lock(&ui_mtx);
	atomic(true, false);

	/* create string */
	while(1){
		va_copy(tlst, lst);
		len = vsnprintf(ostr, ostr_len, fmt, lst);
		va_copy(lst, tlst);

		if(len < ostr_len)
			break;

		ostr_len *= 2;

		delete ostr;
		ostr = new char[ostr_len];
	}

	/* insert text and update cursor position */
	action(FCT, "insert", win, 0, 0, "%d \"%s\"", buf->len, ostr);
	buf->len += len;

	/* update cursor */
	if(cursor_update)
		action(CMD, "setDot", win, 0, 0, "%d", buf->len - 1);

	atomic(false, false);
	pthread_mutex_unlock(&ui_mtx);
}

void vimui::win_clear(int win){
	int len;
	buffer_t* buf;


	buf = MAP_LOOKUP_SAFE(bufid_map, win, buf_mtx);

	if(buf == 0)
		return;

	pthread_mutex_lock(&ui_mtx);

	atomic(true, false);

	/* get length of buffer and remove text */
	if(action(FCT, "getLength", win, result_to_length, (void*)&len, "") == 0)
		action(FCT, "remove", win, 0, 0, "0 %d", len);

	buf->len = 0;

	atomic(false, false);

	pthread_mutex_unlock(&ui_mtx);
}

int vimui::reply(int seq_num, vim_result_t* rlst){
	pthread_mutex_lock(&resp_mtx);

	/* check if this is the sequence number waited for, if not drop it
	 *	this is safe since there can only be one outstanding action()
	 */
	if(resp.seq_num == seq_num){
		VIM("vim reply to seq-num %d\n", seq_num);
		resp.result = rlst;

		pthread_cond_signal(&resp_avail);
	}
	else{
		VIM("drop vim reply with sequence number %d, expected %d\n", seq_num, resp.seq_num);
		vim_result_free(rlst);
	}

	pthread_mutex_unlock(&resp_mtx);

	return 0;
}

int vimui::event(int buf_id, int seq_num, vim_event_id_t evt_id, vim_result_t* rlst){
	response_t* e;


	pthread_mutex_lock(&event_mtx);

	/* check event type */
	if(evt_id & (E_KEYATPOS | E_FILEOPENED | E_KILLED | E_DISCONNECT)){
		e = new response_t;
		e->evt_id = evt_id;
		e->buf_id = buf_id;
		e->result = rlst;
		list_add_tail(&event_lst, e);

		pthread_cond_signal(&event_avail);
	}
	else{
		// drop the result otherwise
		VIM("drop vim event %d for buffer %d, sequence number %d\n", evt_id, buf_id, seq_num);
		vim_result_free(rlst);
	}

	pthread_mutex_unlock(&event_mtx);

	return 0;
}

int vimui::atomic(bool en, bool apply){
	static bool volatile in_atomic = false;
	static vim_cursor_t volatile cur;


	/* only apply changes if not already in atomic or apply is set */
	if(in_atomic && !apply)
		return 0;

	pthread_mutex_lock(&ui_mtx);

	if(en){
		if(action(CMD, "startAtomic", 0, 0, 0, "") != 0)
			goto err;

		/* get current vim buffer and cursor position */
		if(action(FCT, "getCursor", 0, result_to_cursor, (void*)&cur, "") != 0)
			goto err;

		if(apply){
			in_atomic = true;
			cursor_update = false;	// avoid cursor updates within atomic
									// since this breaks netbeans atomicity
		}
	}
	else{
		/* reset cursor to previous buffer if its different from target buffer */
		if(cur.bufid > 0){
			if(action(CMD, "setDot", cur.bufid, 0, 0, "%d/%d", cur.line, cur.column) != 0)
				goto err;
		}

		if(action(CMD, "endAtomic", 0, 0, 0, "") != 0)
			goto err;

		if(apply){
			in_atomic = false;
			cursor_update = true;
		}
	}

	pthread_mutex_unlock(&ui_mtx);
	return 0;

err:
	pthread_mutex_unlock(&ui_mtx);
	return -1;
}

int vimui::action(action_t type, const char* action, int buf_id, int (*process)(vim_result_t*, void*), void* result, const char* fmt, ...){
	static int volatile seq_num = 1;
	static char* volatile s = 0;
	static unsigned int volatile s_len = 0;
	unsigned int i;
	va_list lst;


	if(buf_id < 0)
		return -1;

	va_start(lst, fmt);

	/* send action header */
	nbclient->send(itoa(buf_id, (char**)&s, (unsigned int*)&s_len));
	nbclient->send((char*)":");
	nbclient->send((char*)action);

	VIM("%s\n", action);

	if(type == FCT)			nbclient->send((char*)"/");
	else if(type == CMD)	nbclient->send((char*)"!");

	seq_num++;
	nbclient->send(itoa(seq_num, (char**)&s, (unsigned int*)&s_len));

	/* process arguments */
	if(strlen(fmt) > 0)
		nbclient->send((char*)" ");

	for(i=0; i<strlen(fmt); i++){
		if(fmt[i] == '%'){
			switch(fmt[++i]){
			case 'i':
			case 'd':
				nbclient->send(itoa((int)va_arg(lst, int), (char**)&s, (unsigned int*)&s_len));
				break;

			case 'u':
				nbclient->send(itoa((unsigned int)va_arg(lst, unsigned int), (char**)&s, (unsigned int*)&s_len));
				break;

			case 's':
				nbclient->send(strescape(va_arg(lst, char*), (char**)&s, (unsigned int*)&s_len));
				break;

			default:
				va_end(lst);
				return -1;
			};
		}
		else
			nbclient->send((void*)(fmt + i), 1);

	}

	va_end(lst);

	/* wait for vim response if action is a function */
	if(type == FCT){
		pthread_mutex_lock(&resp_mtx);

		memset((void*)&resp, -1, sizeof(response_t));

		resp.seq_num = seq_num;
		nbclient->send((char*)"\n");		// issue action after mutex has been locked
											// avoid vim response before being ready

		// wait for response
		pthread_cond_wait(&resp_avail, &resp_mtx);

		pthread_mutex_unlock(&resp_mtx);

		// process result if requested, otherwise free
		if(process){
			if(process(resp.result, result) != 0){
				vim_result_free(resp.result);
				return -1;
			}
		}

		vim_result_free(resp.result);
	}
	else
		nbclient->send((char*)"\n");

	return 0;
}

void* vimui::readline_thread(void* arg){
	char c;
	char* line;
	unsigned int i, line_len;
	vimui* vim;


	vim = (vimui*)arg;
	line_len = 255;
	line = (char*)malloc(line_len * sizeof(char));

	i = 0;

	/* read from netbeans socket */
	while(vim->nbclient->recv(&c, 1) > 0){
		if(c == '\r')
			continue;

		line[i++] = c;

		if(i >= line_len){
			line_len *= 2;
			line = (char*)realloc(line, line_len * sizeof(char));
		}

		if(c == '\n'){
			line[i] = 0;

			line[i - 1] = 0;
			VIM("parse vim input \"%s\"\n", line);
			line[i - 1] = '\n';

			// parse line
			VIM("parser return value: %d\n", vimparse(line, vim));

			i = 0;
		}
	}

	/* trigger shutdown of main thread */
	vim->event(0, 0, E_DISCONNECT, 0);

	pthread_exit(0);
}

#include <config/config.h>
#include <common/string.h>
#include <common/log.h>
#include <common/list.h>
#include <common/opt.h>
#include <common/map.h>
#include <gui/vim/vimui.h>
#include <gui/vim/cursor.h>
#include <gui/vim/lexer.lex.h>
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

	seq_num = 1;
	reply = 0;

	event_lst = 0;
	list_init(event_lst);

	pthread_mutex_init(&buf_mtx, 0);
	pthread_mutex_init(&reply_mtx, 0);
	pthread_mutex_init(&event_mtx, 0);
	pthread_cond_init(&reply_avail, 0);
	pthread_cond_init(&event_avail, 0);

	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&ui_mtx, &attr);
	pthread_mutexattr_destroy(&attr);
}

vimui::~vimui(){
	pthread_cond_destroy(&reply_avail);
	pthread_mutex_destroy(&reply_mtx);
	pthread_mutex_destroy(&event_mtx);
	pthread_mutex_destroy(&buf_mtx);
	pthread_mutex_destroy(&ui_mtx);

	/* do not destroy 'event_avail', since this can cause vimgdb
	 * to hand in case it has been terminated via SIGINT or SIGTERM
	 */
	//pthread_cond_destroy(&event_avail);

}

int vimui::init(){
	char* s;


	ostr_len = 256;
	ostr = new char[ostr_len];

	if(ostr == 0)
		goto err_0;

	nbserver = new socket(CONFIG_VIM_NB_PORT, 0);

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
	delete [] ostr;

err_0:
	return -1;
}

void vimui::destroy(){
	vim_event_t* e;


	/* signal shutdown to vim
	 * this automatically closes the vim readline and event
	 * threads through the associated events
	 */
	if(nbclient){
		nbclient->send((char*)"DETACH\n", 7);

		// give vim some time to send disconnect event
		sleep(1);
	}

	/* cancel readline-thread */
	if(read_tid != 0){
		pthread_cancel(read_tid);
		pthread_join(read_tid, 0);
	}

	/* close sockets */
	delete nbclient;
	delete nbserver;

	/* free memory */
	list_for_each(event_lst, e){
		list_rm(&event_lst, e);
		delete e;
	}

	delete [] ostr;
}

char* vimui::readline(){
	char* line = 0;
	unsigned int len = 0;
	vim_event_t* e;


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
		case E_KEYCOMMAND:
			VIM("handle KEYCOMMAND event\n");

			// KEYCOMMAND events are user commands
			// hence signal availability of a command to readline()
			/* copy the data string into line */
			if(len < strlen(e->data) + 1){
				len += strlen(e->data) + 1;
				line = new char[len];

				if(line == 0)
					goto end;
			}

			strcpy(line, e->data);
			goto end;
		
		case E_FILEOPENED:
			VIM("handle FILEOPENED event\n");

			win_create(e->data);
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

		delete e;
	}

end:
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
		action(CMD, "editFile", id, 0, "\"%s\"", name);
	else
		action(CMD, "putBufferNumber", id, 0, "\"%s/%s\"", cwd, name);

	action(CMD, "stopDocumentListen", id, 0, "");

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
			delete [] s;
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
	if(action(CMD, "close", win, 0, "") != 0)
		goto err_0;

	/* remove buffer */
	pthread_mutex_lock(&buf_mtx);

	it_id = bufid_map.find(win);

	if(it_id == bufid_map.end())
		goto err_1;

	it_name = bufname_map.find(it_id->second->name);

	delete [] it_id->second->name;
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
		if(action(CMD, "defineAnnoType", win, 0, "0 \"%s\" \"\" \"%s\" %s %s", key.c_str(), sign, color_fg, color_bg) != 0)
			goto err;

		buf->anno_types[key] = buf->anno_types.size();	// first element has value 1, since anno_types size
														// is incremented by using the []-operator
		annotype = MAP_LOOKUP(buf->anno_types, key);
	}

	/* add annotation */
	if(action(CMD, "addAnno", win, 0, "%d %d %d/0", id, annotype, line) == 0){
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
		if(action(CMD, "removeAnno", win, 0, "%d", anno->second) != 0)
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

		if(buf != 0)	r = action(CMD, "setDot", win, 0, "%d", buf->len - 1);
		else			r = -1;
	}
	else
		r = action(CMD, "setDot", win, 0, "%d/0", line);

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

		delete [] ostr;
		ostr = new char[ostr_len];
	}

	/* insert text and update cursor position */
	action(FCT, "insert", win, 0, "%d \"%s\"", buf->len, ostr);
	buf->len += len;

	/* update cursor */
	if(cursor_update)
		action(CMD, "setDot", win, 0, "%d", buf->len - 1);

	atomic(false, false);
	pthread_mutex_unlock(&ui_mtx);
}

void vimui::win_clear(int win){
	buffer_t* buf;


	buf = MAP_LOOKUP_SAFE(bufid_map, win, buf_mtx);

	if(buf == 0)
		return;

	pthread_mutex_lock(&ui_mtx);

	atomic(true, false);

	/* clear buffer */
	action(FCT, "remove", win, 0, "0 %d", buf->len);
	buf->len = 0;

	atomic(false, false);

	pthread_mutex_unlock(&ui_mtx);
}

int vimui::proc_reply(int seq_num, vim_reply_t* r){
	pthread_mutex_lock(&reply_mtx);

	/* check if this is the sequence number waited for, if not drop it
	 *	this is safe since there can only be one outstanding action()
	 */
	if(this->seq_num == seq_num){
		VIM("vim reply to seq-num %d\n", seq_num);
		reply = r;

		pthread_cond_signal(&reply_avail);
	}
	else{
		VIM("drop vim reply with sequence number %d, expected %d\n", seq_num, this->seq_num);
		delete r;
	}

	pthread_mutex_unlock(&reply_mtx);

	return 0;
}

int vimui::proc_event(int buf_id, vim_event_t* e){
	pthread_mutex_lock(&event_mtx);

	/* check event type */
	if(e->evt_id & (E_KEYCOMMAND | E_FILEOPENED | E_KILLED | E_DISCONNECT)){
		e->buf_id = buf_id;

		list_add_tail(&event_lst, e);

		pthread_cond_signal(&event_avail);
	}
	else{
		// drop the result otherwise
		VIM("drop vim event %d for buffer %d\n", e->evt_id, buf_id);
		delete e;
	}

	pthread_mutex_unlock(&event_mtx);

	return 0;
}

int vimui::atomic(bool en, bool apply){
	static bool volatile in_atomic = false;
	static volatile vim_cursor_t* volatile cursor = 0;


	/* only apply changes if not already in atomic or apply is set */
	if(in_atomic && !apply)
		return 0;

	pthread_mutex_lock(&ui_mtx);

	if(en){
		if(action(CMD, "startAtomic", 0, 0, "") != 0)
			goto err;

		/* get current vim buffer and cursor position */
		delete cursor;
		cursor = 0;

		if(action(FCT, "getCursor", 0, (vim_reply_t**)&cursor, "") != 0)
			goto err;

		if(apply){
			in_atomic = true;
			cursor_update = false;	// avoid cursor updates within atomic
									// since this breaks netbeans atomicity
		}
	}
	else{
		/* reset cursor to previous buffer if its different from target buffer */
		if(cursor && cursor->bufid > 0){
			if(action(CMD, "setDot", cursor->bufid, 0, "%d/%d", cursor->line, cursor->column) != 0)
				goto err;
		}

		if(action(CMD, "endAtomic", 0, 0, "") != 0)
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

int vimui::action(action_t type, const char* action, int buf_id, vim_reply_t** reply, const char* fmt, ...){
	static char* volatile s = 0;
	static unsigned int volatile s_len = 0;
	unsigned int i;
	va_list lst;


	if(buf_id < 0)
		return -1;

	va_start(lst, fmt);

	/* send action header */
	nbclient->send(itoa(buf_id, (char**)&s, (unsigned int*)&s_len, 10));
	nbclient->send((char*)":");
	nbclient->send((char*)action);

	VIM("%s\n", action);

	if(type == FCT)			nbclient->send((char*)"/");
	else if(type == CMD)	nbclient->send((char*)"!");

	seq_num++;
	nbclient->send(itoa(seq_num, (char**)&s, (unsigned int*)&s_len, 10));

	/* process arguments */
	if(strlen(fmt) > 0)
		nbclient->send((char*)" ");

	for(i=0; i<strlen(fmt); i++){
		if(fmt[i] == '%'){
			switch(fmt[++i]){
			case 'i':
			case 'd':
				nbclient->send(itoa((int)va_arg(lst, int), (char**)&s, (unsigned int*)&s_len, 10));
				break;

			case 'u':
				nbclient->send(itoa((unsigned int)va_arg(lst, unsigned int), (char**)&s, (unsigned int*)&s_len, 10));
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

	/* wait for vim reply if action is a function */
	if(type == FCT){
		pthread_mutex_lock(&reply_mtx);

		nbclient->send((char*)"\n");		// issue action after mutex has been locked
											// avoid vim reply before being ready

		// wait for reply
		pthread_cond_wait(&reply_avail, &reply_mtx);
		pthread_mutex_unlock(&reply_mtx);

		// process result if requested, otherwise free
		if(reply)	*reply = this->reply;
		else		delete this->reply;
	}
	else
		nbclient->send((char*)"\n");

	return 0;
}

void* vimui::readline_thread(void* arg){
	char* line;
	unsigned int line_len, i, r;
	vim_event_t* e;
	vimui* vim;


	vim = (vimui*)arg;
	line_len = 256;
	line = (char*)malloc(line_len);

	i = 0;

	/* read from netbeans socket */
	while((r = vim->nbclient->recv(line + i, line_len - i)) > 0){
		i += r;
		line[i] = 0;

		if(line[i - 1] != '\n'){
			line_len *= 2;
			line = (char*)realloc(line, line_len * sizeof(char));
		}
		else{
			line[i - 1] = 0;
			VIM("parse vim input \"%s\"\n", line);
			line[i - 1] = '\n';

			// parse line
			VIM("parser return value: %d\n", vimparse(line, vim));
			vimlex_destroy();

			i = 0;
		}
	}

	/* trigger shutdown of main thread */
	e = new vim_event_t;
	e->evt_id = E_DISCONNECT;

	vim->proc_event(0, e);

	free(line);
	pthread_exit(0);
}

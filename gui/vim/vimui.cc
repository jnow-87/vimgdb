#include <common/string.h>
#include <common/log.h>
#include <common/list.h>
#include <common/opt.h>
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


vimui::vimui(){
	pthread_mutexattr_t attr;


	main_tid = 0;
	read_tid = 0;
	nbserver = 0;
	nbclient = 0;
	ostr = 0;
	cwd = opt.vim_cwd;

	memset((void*)&resp, -1, sizeof(response_t));

	event_lst = 0;
	list_init(event_lst);

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
	pthread_mutex_destroy(&ui_mtx);
}

int vimui::init(pthread_t main_tid){
	this->main_tid = main_tid;

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


	if(read_tid != 0){
		pthread_cancel(read_tid);
		pthread_join(read_tid, 0);
	}

	list_for_each(event_lst, e)
		list_rm(&event_lst, e);

	delete ostr;
	delete nbclient;
	delete nbserver;
}

char* vimui::readline(){
	char* line = 0;
	unsigned int len = 0;
	response_t* e;


	while(1){
		/* wait for data being read by readline_thread */
		pthread_mutex_lock(&event_mtx);

		if(list_empty(event_lst))
			pthread_cond_wait(&event_avail, &event_mtx);

		e = list_first(event_lst);
		list_rm(&event_lst, e);

		pthread_mutex_unlock(&event_mtx);

		/* process command */
		switch(e->evt_id){
		case E_KEYATPOS:
			DEBUG("handle KEYATPOS event\n");

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
			DEBUG("handle FILEOPENED event\n");

			win_create(e->result->sptr);
			break;

		case E_KILLED:
			DEBUG("handle KILLED event\n");

			win_destroy(e->buf_id);
			break;

		case E_DISCONNECT:
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

int vimui::win_create(const char* name, bool oneline, unsigned int height){
	static int bufid = 1;	// avoid using buf-id 0 for netbeans
	int id;
	buffer_t* b;
	map<string, buffer_t*>::iterator it;


	pthread_mutex_lock(&ui_mtx);

	/* check if buffer for name already exists
	 * 	if so return its id, otherwise
	 * 	create new buffer
	 */
	it = bufname_map.find(name);

	if(it != bufname_map.end()){
		pthread_mutex_unlock(&ui_mtx);
		return it->second->id;
	}

	id = bufid;
	bufid++;

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
		action(CMD, "putBufferNumber", id, 0, "\"%s%s\"", cwd, name);

	action(CMD, "stopDocumentListen", id, 0, "");

	/* set buf_id as used */
	b = new buffer_t;
	b->id = id;
	b->name = new char[strlen(name) + 1];

	strcpy(b->name, name);

	bufname_map[name] = b;
	bufid_map[id] = b;

	pthread_mutex_unlock(&ui_mtx);

	return id;
}

int vimui::win_getid(const char* name){
	return win_create(name);
}

int vimui::win_destroy(int win){
	map<string, buffer_t*>::iterator it_name;
	map<int, buffer_t*>::iterator it_id;


	pthread_mutex_lock(&ui_mtx);

	/* close vim buffer */
	if(action(CMD, "close", win, 0, 0, "") != 0){
		pthread_mutex_unlock(&ui_mtx);
		return -1;
	}

	/* remove buffer */
	it_id = bufid_map.find(win);

	if(it_id == bufid_map.end())
		return -1;

	it_name = bufname_map.find(it_id->second->name);

	delete it_id->second->name;
	delete it_id->second;

	bufname_map.erase(it_name);
	bufid_map.erase(it_id);

	pthread_mutex_unlock(&ui_mtx);

	return 0;
}

int vimui::win_anno_add(int win, int line, const char* sign, const char* color_fg, const char* color_bg){
	static unsigned int id = 1;
	buffer_t* buf;
	string key;
	map<int, buffer_t*>::iterator bit;
	map<string, int>::iterator annotype;


	key = sign;
	key += color_fg;
	key += color_bg;

	pthread_mutex_lock(&ui_mtx);

	bit = bufid_map.find(win);

	if(bit == bufid_map.end())
		goto err;

	buf = bit->second;
	annotype = buf->anno_types.find(key);

	if(action(CMD, "startAtomic", 0, 0, 0, "") != 0)
		goto err;

	/* create annotation type if it doesn't exist */
	if(annotype == buf->anno_types.end()){
		if(action(CMD, "defineAnnoType", win, 0, 0, "0 \"%s\" \"\" \"%s\" %s %s", key.c_str(), sign, color_fg, color_bg) != 0)
			goto err;

		buf->anno_types[key] = buf->anno_types.size();	// first element has value 1, since anno_types size
														// is incremented by using the []-operator
		annotype = buf->anno_types.find(key);
	}

	/* add annotation */
	if(action(CMD, "addAnno", win, 0, 0, "%d %d %d/0", id, annotype->second, line) == 0){
		key = line;
		key += sign;

		buf->annos[key] = id;
		id++;

		action(CMD, "endAtomic", 0, 0, 0, "");

		pthread_mutex_unlock(&ui_mtx);
		return 0;
	}

err:
	action(CMD, "endAtomic", 0, 0, 0, "");
	pthread_mutex_unlock(&ui_mtx);
	return -1;
}

int vimui::win_anno_delete(int win, int line, const char* sign){
	buffer_t* buf;
	string key;
	map<int, buffer_t*>::iterator bit;
	map<string, int>::iterator anno;


	pthread_mutex_lock(&ui_mtx);

	bit = bufid_map.find(win);

	if(bit == bufid_map.end())
		goto err;

	buf = bit->second;
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
	return action(CMD, "setDot", win, 0, 0, "%d/0", line);
}

void vimui::win_print(int win, const char* fmt, ...){
	va_list lst;


	va_start(lst, fmt);
	win_vprint(win, fmt, lst);
	va_end(lst);
}

void vimui::win_vprint(int win, const char* fmt, va_list lst){
	int len;
	vim_cursor_t cur;
	va_list tlst;


	pthread_mutex_lock(&ui_mtx);

	action(CMD, "startAtomic", win, 0, 0, "");

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

	/* get current vim buffer and cursor position */
	if(action(FCT, "getCursor", 0, result_to_cursor, (void*)&cur, "") != 0)
		goto end;

	/* get length of target buffer */
	if(action(FCT, "getLength", win, result_to_length, (void*)&len, "") != 0)
		goto end;

	/* insert text and update cursor position */
	action(FCT, "insert", win, 0, 0, "%d \"%s\"", len, ostr);
	action(CMD, "setDot", win, 0, 0, "%d", len + strlen(ostr) - 1);

	/* reset cursor to previous buffer if its different from target buffer */
	if(cur.bufid > 0 && win != cur.bufid)
		action(CMD, "setDot", cur.bufid, 0, 0, "%d/%d", cur.line, cur.column);

end:
	action(CMD, "endAtomic", win, 0, 0, "");

	pthread_mutex_unlock(&ui_mtx);
}

void vimui::win_clear(int win){
	int len;
	vim_cursor_t cur;


	pthread_mutex_lock(&ui_mtx);

	action(CMD, "startAtomic", 0, 0, 0, "");

	/* get current vim buffer and cursor position */
	if(action(FCT, "getCursor", 0, result_to_cursor, (void*)&cur, "") != 0)
		goto end;

	/* get length of buffer and remove text */
	if(action(FCT, "getLength", win, result_to_length, (void*)&len, "") == 0)
		action(FCT, "remove", win, 0, 0, "0 %d", len);

	/* reset cursor to previous buffer if its different from target buffer */
	if(cur.bufid > 0 && win != cur.bufid)
		action(CMD, "setDot", cur.bufid, 0, 0, "%d/%d", cur.line, cur.column);

end:
	action(CMD, "endAtomic", 0, 0, 0, "");

	pthread_mutex_unlock(&ui_mtx);
}

int vimui::reply(int seq_num, vim_result_t* rlst){
	pthread_mutex_lock(&resp_mtx);

	/* check if this is the sequence number waited for, if not drop it
	 *	this is safe since there can only be one outstanding action()
	 */
	if(resp.seq_num == seq_num){
		DEBUG("vim reply to seq-num %d\n", seq_num);
		resp.result = rlst;

		pthread_cond_signal(&resp_avail);
	}
	else{
		DEBUG("drop vim reply with sequence number %d\n", seq_num);
		vim_result_free(rlst);
	}

	pthread_mutex_unlock(&resp_mtx);

	return 0;
}

int vimui::event(int buf_id, int seq_num, const vim_event_t* evt, vim_result_t* rlst){
	response_t* e;


	pthread_mutex_lock(&event_mtx);

	/* check event type */
	if(evt->id & (E_KEYATPOS | E_FILEOPENED | E_KILLED | E_DISCONNECT)){
		e = new response_t;
		e->evt_id = evt->id;
		e->buf_id = buf_id;
		e->result = rlst;
		list_add_tail(&event_lst, e);

		pthread_cond_signal(&event_avail);
	}
	else{
		// drop the result otherwise
		DEBUG("drop vim event %d for buffer %d, sequence number %d\n", evt->id, buf_id, seq_num);
		vim_result_free(rlst);
	}

	pthread_mutex_unlock(&event_mtx);

	return 0;
}

int vimui::action(action_t type, const char* action, int buf_id, int (*process)(vim_result_t*, void*), void* result, const char* fmt, ...){
	static int seq_num = 1;
	static char* s = 0;
	static unsigned int s_len = 0;
	unsigned int i;
	va_list lst;


	if(buf_id < 0)
		return -1;

	va_start(lst, fmt);

	/* send action header */
	nbclient->send(itoa(buf_id, &s, &s_len));
	nbclient->send((char*)":");
	nbclient->send((char*)action);

	if(type == FCT)			nbclient->send((char*)"/");
	else if(type == CMD)	nbclient->send((char*)"!");

	seq_num++;
	nbclient->send(itoa(seq_num, &s, &s_len));

	/* process arguments */
	if(strlen(fmt) > 0)
		nbclient->send((char*)" ");

	for(i=0; i<strlen(fmt); i++){
		if(fmt[i] == '%'){
			switch(fmt[++i]){
			case 'i':
			case 'd':
				nbclient->send(itoa(va_arg(lst, int), &s, &s_len));
				break;

			case 's':
				nbclient->send(strescape(va_arg(lst, char*), &s, &s_len));
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
	memset((void*)&resp, -1, sizeof(response_t));

	if(type == FCT){
		pthread_mutex_lock(&resp_mtx);

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
	response_t* e;
	vimui* vim;
	sigval v;


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
			DEBUG("parse vim input \"%s\"\n", line);
			line[i - 1] = '\n';

			// parse line
			vimparse(line, vim);

			i = 0;
		}
	}

	pthread_sigqueue(vim->main_tid, SIGTERM, v);
	pthread_exit(0);
}

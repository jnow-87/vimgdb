#include <common/string.h>
#include <common/log.h>
#include <common/list.h>
#include <common/opt.h>
#include <gui/vim/vimui.h>
#include <gui/vim/event.h>
#include <gui/vim/result.h>
#include <cmd.hash.h>
#include <parser.tab.h>
#include <stdlib.h>
#include <string.h>


vimui::vimui(){
	seq_num = 1;
	cwd = opt.vim_cwd;
	memset((void*)&resp, -1, sizeof(response_t));
	event_lst = 0;
	list_init(event_lst);

	bufid = 1;	// avoid using buf-id 0 for netbeans

	pthread_cond_init(&resp_avail, 0);
	pthread_mutex_init(&resp_mtx, 0);
	pthread_cond_init(&event_avail, 0);
	pthread_mutex_init(&ui_mtx, 0);
}

vimui::~vimui(){
	pthread_cond_destroy(&resp_avail);
	pthread_mutex_destroy(&resp_mtx);
	pthread_cond_destroy(&event_avail);
	pthread_mutex_destroy(&ui_mtx);
}

int vimui::init(){
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


	pthread_cancel(read_tid);
	pthread_join(read_tid, 0);

	list_for_each(event_lst, e)
		list_rm(&event_lst, e);

	delete cwd;
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
		pthread_mutex_lock(&resp_mtx);

		if(list_empty(event_lst))
			pthread_cond_wait(&event_avail, &resp_mtx);

		e = list_first(event_lst);
		list_rm(&event_lst, e);

		pthread_mutex_unlock(&resp_mtx);

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

void* vimui::readline_thread(void* arg){
	char c;
	char* line;
	unsigned int i, line_len;
	response_t* e;
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
			DEBUG("parse vim input \"%s\"\n", line);
			line[i - 1] = '\n';

			// parse line
			vimparse(line, vim);

			i = 0;
		}
	}

	return 0;
}

int vimui::win_create(const char* name, bool oneline, unsigned int height){
	int id;
	map<string, int>::iterator it;


	pthread_mutex_lock(&ui_mtx);

	/* check if buffer for name already exists
	 * 	if so return its id, otherwise
	 * 	create new buffer
	 */
	it = bufid_map.find(name);

	if(it != bufid_map.end()){
		pthread_mutex_unlock(&ui_mtx);
		return it->second;
	}

	id = bufid;
	bufid++;

	if(id < 0){
		pthread_mutex_unlock(&ui_mtx);
		return -1;
	}

	/* connect to vim buffer */
	action(CMD, "putBufferNumber", id, 0, "\"%s%s\"", cwd, name);
	action(CMD, "stopDocumentListen", id, 0, "");

	/* set buf_id as used */
	bufid_map[name] = id;

	pthread_mutex_unlock(&ui_mtx);

	return id;
}

int vimui::win_getid(const char* name){
	return win_create(name);
}

int vimui::win_destroy(int win_id){
	map<string, int>::iterator it;


	pthread_mutex_lock(&ui_mtx);

	/* close vim buffer */
	if(action(CMD, "close", win_id, 0, "") != 0){
		pthread_mutex_unlock(&ui_mtx);
		return -1;
	}

	/* remove buffer */
	for(it=bufid_map.begin(); it!=bufid_map.end(); it++){
		if(it->second == win_id){
			bufid_map.erase(it);
			break;
		}
	}

	pthread_mutex_unlock(&ui_mtx);

	return 0;
}

void vimui::win_print(int win_id, const char* fmt, ...){
	va_list lst;


	va_start(lst, fmt);
	win_vprint(win_id, fmt, lst);
	va_end(lst);
}

void vimui::win_vprint(int win_id, const char* fmt, va_list lst){
	int len, buf, buf_line, buf_col;
	vim_result_t* r;
	va_list tlst;


	pthread_mutex_lock(&ui_mtx);

	action(CMD, "startAtomic", 0, 0, "");

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
	if(action(FCT, "getCursor", 0, &r, "") != 0)
		goto end;

	buf = r->num;
	buf_line = r->next->num;
	buf_col = r->next->next->num;
	vim_result_free(r);

	/* get length of target buffer */
	if(action(FCT, "getLength", win_id, &r, "") != 0)
		goto end;

	len = r->num;
	vim_result_free(r);

	/* insert text and update cursor position */
	action(FCT, "insert", win_id, 0, "%d \"%s\"", len, ostr);
	action(CMD, "setDot", win_id, 0, "%d", len + strlen(ostr) - 1);

	/* reset cursor to previous buffer if its different from target buffer */
	if(buf > 0 && win_id != buf)
		action(CMD, "setDot", buf, 0, "%d/%d", buf_line, buf_col);

end:
	action(CMD, "endAtomic", 0, 0, "");

	pthread_mutex_unlock(&ui_mtx);
}

void vimui::win_clear(int win_id){
	int buf, buf_line, buf_col;
	vim_result_t* r;


	pthread_mutex_lock(&ui_mtx);

	action(CMD, "startAtomic", 0, 0, "");

	/* get current vim buffer and cursor position */
	if(action(FCT, "getCursor", 0, &r, "") != 0)
		goto end;

	buf = r->num;
	buf_line = r->next->num;
	buf_col = r->next->next->num;
	vim_result_free(r);

	/* get length of buffer and remove text */
	if(action(FCT, "getLength", win_id, &r, "") == 0)
		action(FCT, "remove", win_id, 0, "0 %d", r->num);

	vim_result_free(r);

	/* reset cursor to previous buffer if its different from target buffer */
	if(buf > 0 && win_id != buf)
		action(CMD, "setDot", buf, 0, "%d/%d", buf_line, buf_col);

end:
	action(CMD, "endAtomic", 0, 0, "");

	pthread_mutex_unlock(&ui_mtx);
}

int vimui::reply(int seq_num, vim_result_t* rlst){
	DEBUG("handle vim reply to seq-num %d\n", seq_num);

	pthread_mutex_lock(&resp_mtx);

	/* check if this is the sequence number waited for, if not drop it
	 *	this is safe since there can only be one outstanding action()
	 */
	if(resp.seq_num == seq_num){
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


	pthread_mutex_lock(&resp_mtx);

	/* check event type */
	if(evt->id & (E_KEYATPOS | E_DISCONNECT)){
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

	pthread_mutex_unlock(&resp_mtx);

	return 0;
}

int vimui::action(action_t type, const char* action, int buf_id, vim_result_t** result, const char* fmt, ...){
	static char* s = 0;
	static unsigned int s_len = 0;
	unsigned int i;
	va_list lst;


	/* init result */
	if(result)
		*result = 0;

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

		// update result if requested, otherwise free
		if(result)	*result = resp.result;
		else		vim_result_free(resp.result);

		pthread_mutex_unlock(&resp_mtx);
	}
	else
		nbclient->send((char*)"\n");

	return 0;
}

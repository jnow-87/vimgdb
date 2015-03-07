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


static void* thread_read(void* arg);


vimui::vimui(){
	seq_num = 1;
	cwd = opt.vim_cwd;
	memset((void*)&resp, -1, sizeof(vim_response_t));

	nbuf = 1;	// to avoid using buf-id 0
	max_buf = 4;

	pthread_cond_init(&resp_avail, 0);
	pthread_mutex_init(&resp_mtx, 0);
	pthread_cond_init(&istr_avail, 0);
	pthread_mutex_init(&ui_mtx, 0);
}

vimui::~vimui(){
	pthread_cond_destroy(&resp_avail);
	pthread_mutex_destroy(&resp_mtx);
	pthread_cond_destroy(&istr_avail);
	pthread_mutex_destroy(&ui_mtx);
}

int vimui::init(){
	buf_ids = (bool*)malloc(max_buf * sizeof(bool));

	if(buf_ids == 0)
		goto err_0;

	memset(buf_ids, false, max_buf * sizeof(bool));

	ostr_len = 255;
	ostr = new char[ostr_len];

	if(ostr == 0)
		goto err_1;

	nbserver = new socket(VIM_NB_PORT, 0);

	if(nbserver == 0)
		goto err_2;

	if(nbserver->init_server(TCP) != 0){
		goto err_3;
	}

	nbclient = nbserver->await_client();

	if(nbclient == 0)
		goto err_3;

	nbclient->set_timeout(0);

	if(pthread_create(&read_tid, 0, thread_read, this) != 0)
		goto err_4;

	if(base_init() != 0)
		goto err_5;

	return 0;

err_5:
	pthread_cancel(read_tid);

err_4:
	delete nbclient;

err_3:
	delete nbserver;

err_2:
	delete ostr;

err_1:
	free(buf_ids);

err_0:
	return -1;
}

void vimui::destroy(){
	base_destroy();

	pthread_cancel(read_tid);

	free(buf_ids);

	delete cwd;
	delete ostr;
	delete nbclient;
	delete nbserver;
}

char* vimui::readline(){
	static char* line = 0;
	static unsigned int len = 0;


	/* wait for data being read by readline_thread */
	pthread_mutex_lock(&resp_mtx);

	pthread_cond_wait(&istr_avail, &resp_mtx);

	if(resp.result == 0){
		pthread_mutex_unlock(&resp_mtx);
		return 0;
	}

	/* copy response string into line */
	if(len < strlen(resp.result->sptr)){
		len += strlen(resp.result->sptr) + 1;
		line = new char[len];

		if(line == 0){
			pthread_mutex_unlock(&resp_mtx);
			return 0;
		}
	}

	strcpy(line, resp.result->sptr);
	vim_result_free(resp.result);

	pthread_mutex_unlock(&resp_mtx);

	return line;
}

int vimui::readline_thread(){
	char c;
	char* line;
	int r;
	unsigned int i, line_len;


	line_len = 255;
	line = (char*)malloc(line_len * sizeof(char));

	i = 0;

	/* read from netbeans socket */
	while(nbclient->recv(&c, 1) > 0){
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
			r = vimparse(line, this);
			DEBUG("parser returned %d\n", r);

			i = 0;
		}
	}

	/* signal closed connection */
	pthread_mutex_lock(&resp_mtx);

	resp.result = 0;
	pthread_cond_signal(&istr_avail);

	pthread_mutex_unlock(&resp_mtx);

	return 0;
}

int vimui::win_create(const char* title, bool oneline, unsigned int height){
	unsigned int i;
	int id;


	pthread_mutex_lock(&ui_mtx);

	if(nbuf + 1 > max_buf){
		buf_ids = (bool*)realloc(buf_ids, max_buf * 2 * sizeof(bool));
		memset(buf_ids + max_buf, 0x0, max_buf * sizeof(bool));

		max_buf *= 2;
	}

	/* select new buffer number */
	// buf-id has to be greater 0
	for(id=1; id<max_buf; id++){
		if(buf_ids[id] == false)
			break;
	}

	if(id == max_buf){
		pthread_mutex_unlock(&ui_mtx);
		return -1;
	}

	/* connect to vim buffer */
	action(CMD, "putBufferNumber", id, 0, "\"%s%s\"", cwd, title);
	action(CMD, "stopDocumentListen", id, 0, "");

	/* set buf_id as used */
	buf_ids[id] = true;
	nbuf++;

	pthread_mutex_unlock(&ui_mtx);

	return id;
}

int vimui::win_destroy(int win_id){
	pthread_mutex_lock(&ui_mtx);

	/* close vim buffer */
	if(action(CMD, "close", win_id, 0, "") != 0){
		pthread_mutex_unlock(&ui_mtx);
		return -1;
	}

	/* set buf_id unsused */
	buf_ids[win_id] = false;
	nbuf--;

	pthread_mutex_unlock(&ui_mtx);

	return 0;
}

void vimui::win_write(int win_id, const char* fmt, ...){
	va_list lst;


	va_start(lst, fmt);
	win_vwrite(win_id, fmt, lst);
	va_end(lst);
}

void vimui::win_vwrite(int win_id, const char* fmt, va_list lst){
	int len, buf, buf_line, buf_col;
	vim_result_t* r;


	pthread_mutex_lock(&ui_mtx);

	/* create string */
	len = vsnprintf(ostr, ostr_len, fmt, lst);

	while(len == ostr_len){
		ostr_len *= 2;

		delete ostr;
		ostr = new char[ostr_len];

		len = vsnprintf(ostr, ostr_len, fmt, lst);
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
	if(win_id != buf)
		action(CMD, "setDot", buf, 0, "%d/%d", buf_line, buf_col);

end:
	pthread_mutex_unlock(&ui_mtx);
}

void vimui::win_clear(int win_id){
	vim_result_t* r;


	pthread_mutex_lock(&ui_mtx);

	/* get length of buffer and remove text */
	if(action(FCT, "getLength", win_id, &r, "") == 0)
		action(FCT, "remove", win_id, 0, "0 %d", r->num);

	vim_result_free(r);

	pthread_mutex_unlock(&ui_mtx);
}

int vimui::action(action_t type, const char* action, int buf_id, vim_result_t** result, const char* fmt, ...){
	unsigned int i;
	char *s;
	va_list lst;


	/* init result */
	if(result)
		*result = 0;

	va_start(lst, fmt);

	/* send action header */
	nbclient->send(itoa(buf_id));
	nbclient->send((char*)":");
	nbclient->send((char*)action);

	if(type == FCT)			nbclient->send((char*)"/");
	else if(type == CMD)	nbclient->send((char*)"!");

	seq_num++;
	nbclient->send(itoa(seq_num));

	/* process arguments */
	if(strlen(fmt) > 0)
		nbclient->send((char*)" ");

	for(i=0; i<strlen(fmt); i++){
		if(fmt[i] == '%'){
			switch(fmt[++i]){
			case 'i':
			case 'd':
				nbclient->send(itoa(va_arg(lst, int)));
				break;

			case 's':
				nbclient->send(strescape(va_arg(lst, char*)));
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

	/* wait for vim response if action is a function or has an event as response */
	memset((void*)&resp, -1, sizeof(vim_response_t));

	if(type == FCT || vim_cmd::lookup(action, strlen(action))->evt_id != E_NONE){
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
	DEBUG("handle vim event %d\n", evt->id);

	pthread_mutex_lock(&resp_mtx);

	/* check event type */
	if(evt->id == E_KEYATPOS){
		// KEYATPOS events are user commands
		// hence signal availability of a command to readline()
		resp.result = rlst;
		pthread_cond_signal(&istr_avail);
	}
	else if(resp.seq_num == seq_num){
		// if the sequence number is awaited as action response return the result
		resp.buf_id = buf_id;
		resp.evt_id = evt->id;
		resp.result = rlst;

		pthread_cond_signal(&resp_avail);
	}
	else{
		// drop the result otherwise
		DEBUG("drop vim event %d for buffer %d, sequence number %d\n", evt->id, buf_id, seq_num);
		vim_result_free(rlst);
	}

	pthread_mutex_unlock(&resp_mtx);

	return 0;
}

void* thread_read(void* arg){
	((vimui*)arg)->readline_thread();
	return 0;
}

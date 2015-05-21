#include <common/defaults.h>
#include <common/log.h>
#include <common/pty.h>
#include <common/string.h>
#include <common/file.h>
#include <common/list.h>
#include <gui/gui.h>
#include <gdb/gdb.h>
#include <gdb/frame.h>
#include <gdb/parser.tab.h>
#include <user_cmd/cmd.hash.h>
#include <user_cmd/subcmd.hash.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <unistd.h>


/* global variables */
gdbif* gdb = 0;


/* class definition */
/**
 * \brief	standard constructor
 */
gdbif::gdbif(){
	read_tid = 0;
	event_tid = 0;
	gdb = 0;
	token = 1;
	is_running = false;
	event_lst = 0;
	stop_hdlr_lst = 0;
	exit_hdlr_lst = 0;
	cur_thread = 0;
	gdb_term = 0;

	pthread_mutex_init(&resp_mtx, 0);
	pthread_mutex_init(&event_mtx, 0);
	pthread_cond_init(&resp_avail, 0);
	pthread_cond_init(&event_avail, 0);
}

/**
 * \brief	standard desctructor
 */
gdbif::~gdbif(){
	event_hdlr_t* e;
	sigval v;


	if(gdb_term)
		gdb_term->close();

	/* join readline-thread
	 *	terminated once the gdb terminal is closed
	 */
	if(read_tid != 0){
		do{
			pthread_sigqueue(read_tid, SIGINT, v);
			usleep(1000);
		}while(pthread_tryjoin_np(read_tid, 0) != 0);
	}

	/* join event-thread
	 *	terminated through 'RC_EXIT' event issued by readline-thread
	 */
	if(event_tid != 0){
		do{
			pthread_sigqueue(event_tid, SIGINT, v);
			usleep(1000);
		}while(pthread_tryjoin_np(event_tid, 0) != 0);
	}

	/* close gdb terminal */
	delete gdb_term;
	gdb_term = 0;

	/* clear event handler lists */
	list_for_each(stop_hdlr_lst, e){
		list_rm(&stop_hdlr_lst, e);
		delete e;
	}

	list_for_each(exit_hdlr_lst, e){
		list_rm(&exit_hdlr_lst, e);
		delete e;
	}

	/* clear pthread structures */
	pthread_cond_destroy(&resp_avail);
	pthread_cond_destroy(&event_avail);
	pthread_mutex_destroy(&resp_mtx);
	pthread_mutex_destroy(&event_mtx);
}

/**
 * \brief	exec gdb and initialise its controlling terminal
 *
 * \return	0	on success
 * 			-1	on error (check errno)
 */
int gdbif::init(){
	// initialise pseudo terminal
	gdb_term = new pty();

	// fork child process
	gdb_pid = gdb_term->fork();

	if(gdb_pid == 0){
		/* child */
		log::cleanup();

		return execl(GDB_CMD, GDB_CMD, GDB_ARGS, (char*)0);
	}
	else if(gdb_pid > 0){
		/* parent */
		if(pthread_create(&read_tid, 0, readline_thread, 0) != 0)
			return -1;

		thread_name[read_tid] = "gdb-readline";

		if(pthread_create(&event_tid, 0, event_thread, 0) != 0)
			return -1;

		thread_name[event_tid] = "gdb-event";
		return 0;
	}
	else{
		/* error */
		return -1;
	}
}

void gdbif::on_stop(int (*hdlr)(void)){
	event_hdlr_t* e;


	e = new event_hdlr_t;
	e->hdlr = hdlr;

	list_add_tail(&stop_hdlr_lst, e);
}

void gdbif::on_exit(int (*hdlr)(void)){
	event_hdlr_t* e;


	e = new event_hdlr_t;
	e->hdlr = hdlr;

	list_add_tail(&exit_hdlr_lst, e);
}

/**
 * \brief	create gdb machine interface (MI) command
 *
 * \param	cmd		target command
 * \param	fmt		printf-like format string, describing the following parameters
 * 					supported identifiers:
 * 						'%d'		integer
 * 						'%s'		string
 * 						'%ss %d'	array of strings, length is defined by the
 * 									following integer
 * 						'--'		separator between options and parameters
 * 						.			everything else except blanks is printed literaly
 *
 * \param	...		parameters according to param_fmt
 *
 * \return	>0		token used for the command
 * 			-1		error
 */
int gdbif::mi_issue_cmd(char* cmd, gdb_result_class_t ok_mask, int(*process)(gdb_result_t*, void**), void** r, const char* fmt, ...){
	static char* volatile s = 0;
	static unsigned int volatile s_len = 0;
	static pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
	unsigned int i, j, argc;
	char** argv;
	bool quoted;
	va_list lst;
	response_t resp;


	pthread_mutex_lock(&m);

	va_start(lst, fmt);

	GDB("issue: %s\n", cmd);

	gdb_term->write(itoa(token, (char**)&s, (unsigned int*)&s_len));
	gdb_term->write((char*)"-");
	gdb_term->write(cmd);

	if(*fmt != 0)
		gdb_term->write((char*)" ");

	for(i=0; i<strlen(fmt); i++){
		switch(fmt[i]){
		case '%':
			switch(fmt[i + 1]){
			case 'd':
				gdb_term->write(itoa((int)va_arg(lst, int), (char**)&s, (unsigned int*)&s_len));
				i++;
				break;

			case 'u':
				gdb_term->write(itoa((unsigned int)va_arg(lst, unsigned int), (char**)&s, (unsigned int*)&s_len));
				i++;
				break;

			case 's':
				if(strncmp(fmt + i + 2, "s %d", 4) == 0 || strncmp(fmt + i + 2, "sq %d", 5) == 0){
					argv = va_arg(lst, char**);
					argc = va_arg(lst, int);
					quoted = fmt[i + 3] == 'q' ? true : false;

					for(j=0; j<argc; j++){
						if(j != 0)	gdb_term->write((char*)" ");
						if(quoted)	gdb_term->write((char*)"\"");

						gdb_term->write(argv[j]);

						if(quoted)	gdb_term->write((char*)"\"");
					}

					i += quoted ? 5 : 4;
				}
				else
					gdb_term->write(va_arg(lst, char*));

				i++;
				break;

			default:
				pthread_mutex_unlock(&m);
				va_end(lst);

				ERROR("invalid format sequence %%%c\n", fmt[i + 1]);
				return -1;
			};

			break;

		default:
			gdb_term->write((void*)(fmt + i), 1);
			break;
		};
	}

	/* wait for gdb response */
	pthread_mutex_lock(&resp_mtx);

	memset((void*)&resp, 0x0, sizeof(response_t));

	gdb_term->write((char*)"\n");	// ensure that response cannot arrive
									// before it is expected

	pthread_cond_wait(&resp_avail, &resp_mtx);

	// make local copy to allow unlocking mutex
	resp = this->resp;
	token++;

	pthread_mutex_unlock(&resp_mtx);
	pthread_mutex_unlock(&m);

	va_end(lst);

	/* process response */
	if((resp.rclass & ok_mask)){
		if(resp.result){
			if(process){
				if(process(resp.result, r) != 0){
					ERROR("unable to process result for \"%s\"\n", cmd);
					resp.rclass = RC_ERROR;
				}
			}
			else if(r){
				*r = (void**)resp.result;
				resp.result = 0;
			}
		}
	}
	else
		USER("gdb-error %s: \"%s\"\n", cmd, (resp.result ? resp.result->value->value : "gdb implementation error"));

	gdb_result_free(resp.result);

	if(resp.rclass & ok_mask)
		return 0;
	return -1;
}

int gdbif::mi_proc_result(gdb_result_class_t rclass, unsigned int token, gdb_result_t* result){
	pthread_mutex_lock(&resp_mtx);

	if(this->token != token)
		ERROR("result token (%d) doesn't match issued token (%d)\n", token, this->token);

	resp.result = result;
	resp.rclass = rclass;

	pthread_cond_signal(&resp_avail);
	pthread_mutex_unlock(&resp_mtx);

	return 0;
}

int gdbif::mi_proc_async(gdb_result_class_t rclass, unsigned int token, gdb_result_t* result){
	response_t* e;


	pthread_mutex_lock(&event_mtx);

	switch(rclass){
	case RC_EXIT:
	case RC_STOPPED:
	case RC_RUNNING:
		e = new response_t;

		e->rclass = rclass;
		e->result = result;
		list_add_tail(&event_lst, e);

		pthread_cond_signal(&event_avail);
		break;

	default:
		GDB("unhandled gdb-event %d\n", rclass);
		gdb_result_free(result);
	};

	pthread_mutex_unlock(&event_mtx);

	return 0;
}

int gdbif::mi_proc_stream(gdb_stream_class_t sclass, char* stream){
	USER("%s", strdeescape(stream));	// use "%s" to avoid issues with '%' within stream
	return 0;
}

/**
 * \brief	read from gdb terminal
 *
 * \param	buf		target buffer
 * \param	nbytes	max bytes to read
 *
 * \return	number of read bytes on success
 * 			-1 on error
 */
int gdbif::read(void* buf, unsigned int nbytes){
	return gdb_term->read(buf, nbytes);
}

/**
 * \brief	write to gdb terminal
 *
 * \param	buf		source buffer
 * \param	nbytes	number of bytes to write
 *
 * \return	number of written bytes on success
 * 			-1 on error
 */
int gdbif::write(void* buf, unsigned int nbytes){
	return gdb_term->write(buf, nbytes);
}

int gdbif::sigsend(int sig){
	sigval v;

	return sigqueue(gdb_pid, sig, v);
}

bool gdbif::running(){
	return is_running;
}

bool gdbif::running(bool state){
	return (is_running = state);
}

unsigned int gdbif::threadid(){
	return cur_thread;
}

void* gdbif::readline_thread(void* arg){
	char c, *line;
	unsigned int i, len;


	i = 0;

	len = 255;
	line = (char*)malloc(len * sizeof(char));

	if(line == 0)
		goto err_0;

#ifdef GUI_CURSES
	if(ui->win_create("gdb-log", true, 0) < 0)
		goto err_1;
#endif // GUI_CURSES

	while(1){
		if(gdb->read(&c, 1) == 1){
			// ignore CR to avoid issues when printing the string
			if(c == '\r')
				continue;

			line[i++] = c;

			if(i >= len){
				len *= 2;
				line = (char*)realloc(line, len);

				if(line == 0)
					goto err_2;
			}

			// check for end of gdb line, a simple newline as separator
			// doesn't work, since the parse would try to parse the line,
			// detecting a syntax error
			if(strncmp(line + i - 6, "(gdb)\n", 6) == 0 ||
			   strncmp(line + i - 7, "(gdb) \n", 7) == 0
			  ){
				line[i] = 0;

				GDB("parse gdb string \"%.10s\"\n", line);
				ui->win_print(ui->win_getid(GDBLOG_NAME), "%s", line);		// use "%s" to avoid issues with '%' within line

				i = gdbparse(line, gdb);

				GDB("parser return value: %d\n", i);
				ui->win_print(ui->win_getid(GDBLOG_NAME), "parser return value: %d\n", i);

				i = 0;
			}
		}
		else{
			GDB("gdb read shutdown\n");
			break;
		}
	}

err_2:
	ui->win_destroy(ui->win_getid(GDBLOG_NAME));

#ifdef GUI_CURSES
err_1:
#endif // GUI_CURSES

	free(line);

err_0:
	/* trigger shutdown */
	// force event-thread to exit
	gdb->mi_proc_async(RC_EXIT, 0, 0);

	// exit
	pthread_exit(0);
}

void* gdbif::event_thread(void* arg){
	int r;
	response_t* e;
	event_hdlr_t* ehdlr;


	while(1){
		pthread_mutex_lock(&gdb->event_mtx);

		if(list_empty(gdb->event_lst))
			pthread_cond_wait(&gdb->event_avail, &gdb->event_mtx);

		e = list_first(gdb->event_lst);
		list_rm(&gdb->event_lst, e);

		pthread_mutex_unlock(&gdb->event_mtx);

		switch(e->rclass){
		case RC_EXIT:
			list_for_each(gdb->exit_hdlr_lst, ehdlr){
				if(ehdlr->hdlr() != 0)
					USER("error executing on-stop handler\n");
			}

			gdb_result_free(e->result);
			delete e;

			pthread_exit(0);

		case RC_STOPPED:
			GDB("handle event STOPPED\n");
			r = gdb->evt_stopped(e->result);
			break;

		case RC_RUNNING:
			GDB("handle event RUNNING\n");
			r = gdb->evt_running(e->result);
			break;

		default:
			r = -1;
		};

		if(r != 0)
			ERROR("error handling gdb-event %d\n", e->rclass);

		gdb_result_free(e->result);
		delete e;
	}
}

int gdbif::evt_running(gdb_result_t* result){
	running(true);
	return 0;
}

int gdbif::evt_stopped(gdb_result_t* result){
	char* reason;
	gdb_result_t* r;
	gdb_frame_t* frame;
	event_hdlr_t* e;


	reason = 0;
	frame = 0;

	running(false);

	/* parse result */
	list_for_each(result, r){
		switch(r->var_id){
		case IDV_REASON:
			reason = (char*)r->value->value;
			break;

		case IDV_FRAME:
			gdb_frame_t::result_to_frame((gdb_result_t*)r->value->value, &frame);
			break;

		case IDV_THREAD_ID:
			cur_thread = atoi((char*)r->value->value);
			break;

		default:
			break;
		};
	}

	if(reason == 0)
		goto err;

	/* check reason */
	if(reason != 0 && frame != 0){
		if(FILE_EXISTS(frame->fullname)){
			ui->win_anno_add(ui->win_create(frame->fullname), frame->line, "ip", "White", "Black");
			ui->win_cursor_set(ui->win_create(frame->fullname), frame->line);
		}
		else
			USER("file \"%s\" does not exist\n", frame->fullname);
	}
	else if(strcmp(reason, "exited-normally") == 0){
		USER("program exited\n");
		goto end;
	}

	/* update variables */
	gdb_variable_t::get_changed();

	/* execute callbacks */
	list_for_each(stop_hdlr_lst, e){
		if(e->hdlr() != 0)
			USER("error executing on-stop handler\n");
	}

end:
	delete frame;
	return 0;

err:
	delete frame;
	return -1;
}

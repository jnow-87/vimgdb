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


/* class definition */
/**
 * \brief	standard constructor
 */
gdbif::gdbif(){
	main_tid = 0;
	read_tid = 0;
	gdb = 0;
	token = 1;
	is_running = false;
	event_lst = 0;
	stop_hdlr = 0;

	pthread_mutex_init(&resp_mtx, 0);
	pthread_mutex_init(&event_mtx, 0);
	pthread_cond_init(&resp_avail, 0);
	pthread_cond_init(&event_avail, 0);
}

/**
 * \brief	standard desctructor
 */
gdbif::~gdbif(){
	// close gdb terminal
	delete this->gdb;
	this->gdb = 0;

	if(read_tid != 0){
		pthread_cancel(read_tid);
		pthread_join(read_tid, 0);
	}

	if(event_tid != 0){
		pthread_cancel(event_tid);
		pthread_join(event_tid, 0);
	}

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
int gdbif::init(pthread_t main_tid){
	this->main_tid = main_tid;

	// initialise pseudo terminal
	this->gdb = new pty();

	// fork child process
	gdb_pid = gdb->fork();

	if(gdb_pid == 0){
		/* child */
		log::cleanup();

		return execl(GDB_CMD, GDB_CMD, GDB_ARGS, (char*)0);
	}
	else if(gdb_pid > 0){
		/* parent */
		if(pthread_create(&read_tid, 0, readline_thread, this) != 0)
			return -1;

		thread_name[read_tid] = "gdb-readline";

		if(pthread_create(&event_tid, 0, event_thread, this) != 0)
			return -1;

		thread_name[event_tid] = "gdb-event";
		return 0;
	}
	else{
		/* error */
		return -1;
	}
}

void gdbif::on_stop(int (*hdlr)(gdbif*)){
	stop_hdlr_t* e;


	e = new stop_hdlr_t;
	e->hdlr = hdlr;

	list_add_tail(&stop_hdlr, e);
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
	unsigned int i, j, argc;
	char** argv;
	va_list lst;


	va_start(lst, fmt);

	gdb->write(itoa(token, (char**)&s, (unsigned int*)&s_len));
	gdb->write((char*)"-");
	gdb->write(cmd);

	if(*fmt != 0)
		gdb->write((char*)" ");

	for(i=0; i<strlen(fmt); i++){
		switch(fmt[i]){
		case '%':
			switch(fmt[i + 1]){
			case 'd':
				gdb->write(itoa(va_arg(lst, int), (char**)&s, (unsigned int*)&s_len));
				i++;
				break;

			case 's':
				if(strncmp(fmt + i + 2, "s %d", 4) == 0){
					argv = va_arg(lst, char**);
					argc = va_arg(lst, int);

					for(j=0; j<argc; j++)
						gdb->write(argv[j]);

					i += 4;
				}
				else
					gdb->write(va_arg(lst, char*));

				i++;
				break;

			default:
				ERROR("invalid format sequence %%%c\n", fmt[i + 1]);
				va_end(lst);
				return -1;
			};

			break;

		default:
			gdb->write((void*)(fmt + i), 1);
			break;
		};
	}

	/* wait for gdb response */
	pthread_mutex_lock(&resp_mtx);

	memset((void*)&resp, 0x0, sizeof(response_t));

	gdb->write((char*)"\n");	// ensure that response cannot arrive
								// before it is expected

	pthread_cond_wait(&resp_avail, &resp_mtx);

	token++;

	if((resp.rclass & ok_mask)){
		if(resp.result && r && process){
			if(process(resp.result, r) != 0){
				ERROR("unable to process result for \"%s\"\n", cmd);
				resp.rclass = RC_ERROR;
			}
		}
	}
	else
		USER("gdb-error %s: \"%s\"\n", cmd, resp.result->value->value);

	gdb_result_free(resp.result);

	pthread_mutex_unlock(&resp_mtx);

	va_end(lst);

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
	int r;
	response_t* e;


	pthread_mutex_lock(&event_mtx);

	switch(rclass){
	case RC_STOPPED:
	case RC_RUNNING:
		e = new response_t;

		e->rclass = rclass;
		e->result = result;
		list_add_tail(&event_lst, e);

		pthread_cond_signal(&event_avail);
		break;

	default:
		DEBUG("unhandled gdb-event %d\n", rclass);
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
	return gdb->read(buf, nbytes);
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
	return gdb->write(buf, nbytes);
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

void* gdbif::readline_thread(void* arg){
	char c, *line;
	int win_id_gdb;
	unsigned int i, len;
	gdbif* gdb;
	sigval v;


	i = 0;
	gdb = (gdbif*)arg;

	len = 255;
	line = (char*)malloc(len * sizeof(char));

	if(line == 0)
		goto err_0;

	win_id_gdb = ui->win_create("gdb-log", true, 0);

	if(win_id_gdb < 0)
		goto err_1;

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
					goto err_0;
			}

			// check for end of gdb line, a simple newline as separator
			// doesn't work, since the parse would try to parse the line,
			// detecting a syntax error
			if(strncmp(line + i - 6, "(gdb)\n", 6) == 0 ||
			   strncmp(line + i - 7, "(gdb) \n", 7) == 0
			  ){
				line[i] = 0;

				DEBUG("parse gdb string \"%.10s\"\n", line);
				ui->win_print(win_id_gdb, "%s", line);		// use "%s" to avoid issues with '%' within line

				i = gdbparse(line, gdb);

				DEBUG("parser return value: %d\n", i);
				ui->win_print(win_id_gdb, "parser return value: %d\n", i);

				i = 0;
			}
		}
		else{
			INFO("gdb read shutdown\n");
			break;
		}
	}

err_2:
	ui->win_destroy(win_id_gdb);

err_1:
	free(line);

err_0:
	pthread_sigqueue(gdb->main_tid, SIGTERM, v);
	pthread_exit(0);

}

void* gdbif::event_thread(void* arg){
	int r;
	gdbif* gdb;
	response_t* e;


	gdb = (gdbif*)arg;

	while(1){
		pthread_mutex_lock(&gdb->event_mtx);

		if(list_empty(gdb->event_lst))
			pthread_cond_wait(&gdb->event_avail, &gdb->event_mtx);

		e = list_first(gdb->event_lst);
		list_rm(&gdb->event_lst, e);

		pthread_mutex_unlock(&gdb->event_mtx);

		switch(e->rclass){
		case RC_STOPPED:
			r = gdb->evt_stopped(e->result);
			break;

		case RC_RUNNING:
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
	stop_hdlr_t* e;


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
			conv_frame((gdb_result_t*)r->value->value, &frame);
			break;

		default:
			break;
		};
	}

	if(reason == 0)
		goto err;

	/* check reason */
	if(strcmp(reason, "breakpoint-hit") == 0 ||
	   strcmp(reason, "end-stepping-range") == 0 ||
	   strcmp(reason, "function-finished") == 0){

		if(FILE_EXISTS(frame->fullname)){
			ui->win_cursor_set(ui->win_getid(frame->fullname), frame->line);
			ERROR("add anno\n");
			ui->win_anno_add(ui->win_getid(frame->fullname), frame->line, "ip", "White", "Black");
		}
		else
			USER("file \"%s\" does not exist\n", frame->fullname);
	}
	else if(strcmp(reason, "exited-normally") == 0){
		USER("program exited\n");
	}

	/* execute callbacks */
	list_for_each(stop_hdlr, e){
		if(e->hdlr(this) != 0)
			USER("error executing on-stop handler\n");
	}

	delete frame;
	return 0;

err:
	delete frame;
	return -1;
}

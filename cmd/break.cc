#include <common/log.h>
#include <common/list.h>
#include <gdb/gdb.h>
#include <gdb/result.h>
#include <gui/gui.h>
#include <cmd/cmd.h>
#include <cmd/subcmd.hash.h>
#include <string.h>
#include <stdlib.h>
#include <map>
#include <string>


using namespace std;


/* types */
typedef enum{
	A_ADD = 0,
	A_DELETE,
	A_ENABLE,
	A_DISABLE,
} action_t;

typedef struct{
	char* num;

	unsigned int line;
	char *filename,
		 *fullname,
		 *at;

	bool enabled;
} breakpt_t;

typedef struct{
	action_t action;
	breakpt_t* bkpt;
	map<string, breakpt_t*>::iterator it;
} data_t;


/* static variables */
static map<string, breakpt_t*> breakpt_lst;

static const char* cmd_str[] = {
	"break-insert",
	"break-delete",
	"break-enable",
	"break-disable",
};


/* static prototypes */
void breakpt_read(result_t* result, breakpt_t* bkpt);
void breakpt_print();


/* global functions */
int cmd_break_exec(gdb_if* gdb, int argc, char** argv){
	const struct subcmd_t* scmd;
	data_t* data;
	map<string, breakpt_t*>::iterator it;


	if(argc < 3){
		USER("invalid number of arguments to commands \"%s\"\n", argv[0]);
		cmd_break_help(argv[0]);
		return -1;
	}

	scmd = subcmd::lookup(argv[1], strlen(argv[1]));
	if(scmd == 0){
		USER("invalid sub command \"%s\" to \"%s\"\n", argv[1], argv[0]);
		return -1;
	}

	data = new data_t;

	switch(scmd->id){
	case ADD:
		data->action = A_ADD;
		break;

	case DELETE:
	case ENABLE:
	case DISABLE:
		it = breakpt_lst.find(argv[2]);
		if(it == breakpt_lst.end()){
			USER("error: no breakpoint found for \"%s\"\n", argv[2]);
			goto err;
		}

		if(scmd->id == DELETE)			data->action = A_DELETE;
		else if(scmd->id == ENABLE)		data->action = A_ENABLE;
		else if(scmd->id == DISABLE)	data->action = A_DISABLE;

		data->it = it;
		data->bkpt = it->second;

		argv[2] = it->second->num;
		break;

	default:
		USER("unhandled sub command \"%s\" to \"%s\"\n", argv[1], argv[0]);
		goto err;
	};

	if(gdb->mi_issue_cmd((char*)cmd_str[data->action], 0, 0, argv + 2, 1, cmd_break_resp, (void*)data) < 0){
		WARN("error sending mi command\n");
		goto err;
	}

	return 0;

err:
	delete data;
	return -1;
}

int cmd_break_resp(result_class_t rclass, result_t* result, char* cmdline, void* _data){
	breakpt_t* bkpt;
	data_t* data;
	char key[255];


	data = (data_t*)_data;

	switch(rclass){
	case RC_DONE:
		USER("done: exec \"%s\"\n", cmdline);

		switch(data->action){
		case A_ADD:
			bkpt = new breakpt_t;
			memset(bkpt, 0x0, sizeof(breakpt_t));

			breakpt_read(result, bkpt);

			if(bkpt->filename != 0)	snprintf(key, 255, "%s:%d", bkpt->filename, bkpt->line);
			else					snprintf(key, 255, "%s", bkpt->at);

			breakpt_lst[key] = bkpt;

			breakpt_print();
			break;

		case A_DELETE:
			breakpt_lst.erase(data->it);
			breakpt_print();
			break;

		case A_ENABLE:
			data->bkpt->enabled = true;
			breakpt_print();
			break;

		case A_DISABLE:
			data->bkpt->enabled = false;
			breakpt_print();
			break;
		};

		break;

	case RC_ERROR:
		USER("gdb reported error for command \"%s\"\n\t%s\n", cmdline, result->value->value);
		break;

	default:
		WARN("unhandled result class %d\n", rclass);
		break;
	};

	delete data;

	return 0;
}

void cmd_break_help(char* cmd){
	USER("usage %s <sub-command> <arg>\n", cmd);
	USER("   sub-commands:\n");
	USER("       add <location>\n");
	USER("          add new breakpoint, with location being any of\n");
	USER("             - function\n");
	USER("             - filename:linenum\n");
	USER("             - filename:function\n");
	USER("             - *address\n");
	USER("\n");
	USER("       delete <location>\n");
	USER("          delete breakpoint at <location> as specified in breakpoint window\n");
	USER("\n");
	USER("       enable <location>\n");
	USER("          enable breakpoint at <location> as specified in breakpoint window\n");
	USER("\n");
	USER("       disable <location>\n");
	USER("          disable breakpoint at <location> as specified in breakpoint window\n");
}


/* local functions */
void breakpt_read(result_t* result, breakpt_t* bkpt){
	result_t* r;


	list_for_each(result, r){
		switch(r->var_id){
		case V_NUMBER:
			bkpt->num = new char[strlen((const char*)r->value->value) + 1];
			strcpy(bkpt->num, (const char*)r->value->value);
			break;

		case V_LINE:
			bkpt->line = atoi((char*)r->value->value);
			break;

		case V_FILE:
			bkpt->filename = new char[strlen((const char*)r->value->value) + 1];
			strcpy(bkpt->filename, (const char*)r->value->value);
			break;

		case V_FULLNAME:
			bkpt->fullname = new char[strlen((const char*)r->value->value) + 1];
			strcpy(bkpt->fullname, (const char*)r->value->value);
			break;

		case V_ENABLED:
			bkpt->enabled = (strcmp((const char*)r->value->value, "y") == 0) ? true : false;
			break;

		case V_AT:
			bkpt->at = new char[strlen((const char*)r->value->value) + 1];
			strcpy(bkpt->at, (const char*)r->value->value);
			break;

		default:
			if(r->value->type == RESULT_LIST)
				breakpt_read((result_t*)r->value->value, bkpt);
		};
	}
}

void breakpt_print(){
	map<string, breakpt_t*>::iterator it;
	breakpt_t* bkpt;


	ui->break_clear();

	for(it=breakpt_lst.begin(); it!=breakpt_lst.end(); it++){
		bkpt = it->second;

		if(bkpt->enabled){
			if(bkpt->filename != 0)	ui->break_print("   %s:%d\n", bkpt->filename, bkpt->line);
			else					ui->break_print("   %s\n", bkpt->at);
		}
		else{
			if(bkpt->filename != 0)	ui->break_print("   %s:%d [disabled]\n", bkpt->filename, bkpt->line);
			else					ui->break_print("   %s [disabled]\n", bkpt->at);
		}
	}
}

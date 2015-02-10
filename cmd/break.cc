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
	unsigned int num;

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
int cmd_break_exec(gdbif* gdb, int argc, char** argv){
	unsigned int i;
	int retval;
	const struct subcmd_t* scmd;
	data_t* data;
	map<string, breakpt_t*>::iterator it;
	arglist_t* param;


	if(argc != 3){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_break_help(1, argv);
		return 0;
	}

	scmd = subcmd::lookup(argv[1], strlen(argv[1]));

	if(scmd == 0){
		USER("invalid sub-command \"%s\" to command \"%s\"\n", argv[1], argv[0]);
		return 0;
	}

	data = new data_t;
	param = 0;

	switch(scmd->id){
	case ADD:
		data->action = A_ADD;
		arg_add_string(param, argv[2], false);
		break;

	case DELETE:
	case ENABLE:
	case DISABLE:
		it = breakpt_lst.find(argv[2]);

		if(it == breakpt_lst.end()){
			USER("error: no breakpoint found for \"%s\"\n", argv[2]);
			retval = 0;
			goto err;
		}

		if(scmd->id == DELETE)			data->action = A_DELETE;
		else if(scmd->id == ENABLE)		data->action = A_ENABLE;
		else if(scmd->id == DISABLE)	data->action = A_DISABLE;

		data->it = it;
		data->bkpt = it->second;

		arg_add_int(param, it->second->num, false);
		break;

	default:
		USER("unhandled sub command \"%s\" to \"%s\"\n", argv[1], argv[0]);
		retval = 0;
		goto err;
	};

	if(gdb->mi_issue_cmd((char*)cmd_str[data->action], 0, param, cmd_break_resp, (void*)data) < 0){
		WARN("error sending mi command\n");
		retval = -1;
		goto err;
	}

	arg_clear(param);
	return 0;

err:
	arg_clear(param);
	delete data;
	return retval;
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

void cmd_break_help(int argc, char** argv){
	unsigned int i;
	const struct subcmd_t* scmd;


	if(argc == 1){
		USER("usage %s <sub-command> <arg>\n", argv[0]);
		USER("   sub-commands:\n");
		USER("       add <location>       add breakpoint\n");
		USER("       delete <location>    delete breakpoint\n");
		USER("       enable <location>    enable breakpoint\n");
		USER("       disable <location>   disable breakpoint\n");
		USER("\n");
	}
	else{
		for(i=1; i<argc; i++){
			scmd = subcmd::lookup(argv[i], strlen(argv[i]));

			if(scmd == 0){
				USER("invalid sub-command \"%s\" to command \"%s\"\n", argv[i], argv[0]);
				continue;
			}

			switch(scmd->id){
			case ADD:
				USER("usage %s %s <location>\n", argv[0], argv[i]);
				USER("   add new breakpoint, with location being any of\n");
				USER("      - function\n");
				USER("      - filename:linenum\n");
				USER("      - filename:function\n");
				USER("      - *address\n");
				USER("\n");
				break;

			case DELETE:
				USER("usage %s %s <location>\n", argv[0], argv[i]);
				USER("          delete breakpoint at <location> as specified in breakpoint window\n");
				USER("\n");
				break;

			case ENABLE:
				USER("usage %s %s <location>\n", argv[0], argv[i]);
				USER("          enable breakpoint at <location> as specified in breakpoint window\n");
				USER("\n");
				break;

			case DISABLE:
				USER("usage %s %s <location>\n", argv[0], argv[i]);
				USER("          disable breakpoint at <location> as specified in breakpoint window\n");
				USER("\n");
				break;

			default:
				USER("invalid sub-command \"%s\" to command \"%s\"\n", argv[i], argv[0]);
			};
		}
	}
}


/* local functions */
void breakpt_read(result_t* result, breakpt_t* bkpt){
	result_t* r;


	list_for_each(result, r){
		switch(r->var_id){
		case V_NUMBER:
			bkpt->num = atoi((char*)r->value->value);
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


	ui->clear(WIN_BREAK);

	for(it=breakpt_lst.begin(); it!=breakpt_lst.end(); it++){
		bkpt = it->second;

		if(bkpt->enabled){
			if(bkpt->filename != 0)	ui->print(WIN_BREAK, "   %s:%d\n", bkpt->filename, bkpt->line);
			else					ui->print(WIN_BREAK, "   %s\n", bkpt->at);
		}
		else{
			if(bkpt->filename != 0)	ui->print(WIN_BREAK, "   %s:%d [disabled]\n", bkpt->filename, bkpt->line);
			else					ui->print(WIN_BREAK, "   %s [disabled]\n", bkpt->at);
		}
	}
}

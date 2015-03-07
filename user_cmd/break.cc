#include <common/log.h>
#include <common/list.h>
#include <gdb/gdb.h>
#include <gdb/result.h>
#include <gui/gui.h>
#include <user_cmd/cmd.h>
#include <user_cmd/subcmd.hash.h>
#include <string.h>
#include <stdlib.h>
#include <map>
#include <string>


using namespace std;


/* types */
typedef struct{
	unsigned int num;

	unsigned int line;
	char *filename,
		 *fullname,
		 *at;

	bool enabled;
} breakpt_t;


/* static variables */
static map<string, breakpt_t*> breakpt_lst;

static const char* cmd_str[] = {
	"break-insert",
	"break-delete",
	"break-enable",
	"break-disable",
};


/* static prototypes */
void breakpt_read(gdb_result_t* result, breakpt_t* bkpt);
void breakpt_print();


/* global functions */
int cmd_break_exec(gdbif* gdb, int argc, char** argv){
	char key[255];
	const struct user_subcmd_t* scmd;
	map<string, breakpt_t*>::iterator it;
	gdb_response_t* resp;
	breakpt_t* bkpt;


	if(argc != 3){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_break_help(1, argv);
		return 0;
	}

	scmd = user_subcmd::lookup(argv[1], strlen(argv[1]));

	if(scmd == 0){
		USER("invalid sub-command \"%s\" to command \"%s\"\n", argv[1], argv[0]);
		return 0;
	}

	switch(scmd->id){
	case ADD:
		resp = gdb->mi_issue_cmd((char*)"break-insert", "%s", argv[2]);
		break;

	case DELETE:
	case ENABLE:
	case DISABLE:
		it = breakpt_lst.find(argv[2]);
		bkpt = it->second;

		if(it == breakpt_lst.end()){
			USER("error: no breakpoint found for \"%s\"\n", argv[2]);
			return 0;
		}

		if(scmd->id == DELETE)			resp = gdb->mi_issue_cmd((char*)"break-delete", "%d", it->second->num);
		else if(scmd->id == ENABLE)		resp = gdb->mi_issue_cmd((char*)"break-enable", "%d", it->second->num);
		else if(scmd->id == DISABLE)	resp = gdb->mi_issue_cmd((char*)"break-disable", "%d", it->second->num);
		break;

	default:
		USER("unhandled sub command \"%s\" to \"%s\"\n", argv[1], argv[0]);
		return 0;
	};
	
	if(resp == 0){
		WARN("error issuing mi command\n");
		return -1;
	}

	switch(resp->rclass){
	case RC_DONE:
		USER("done: exec \"%s %s\"\n", argv[0], argv[1]);

		switch(scmd->id){
		case ADD:
			bkpt = new breakpt_t;
			memset(bkpt, 0x0, sizeof(breakpt_t));

			breakpt_read(resp->result, bkpt);

			if(bkpt->filename != 0)	snprintf(key, 255, "%s:%d", bkpt->filename, bkpt->line);
			else					snprintf(key, 255, "%s", bkpt->at);

			breakpt_lst[key] = bkpt;

			breakpt_print();
			break;

		case DELETE:
			delete it->second;
			breakpt_lst.erase(it);
			breakpt_print();
			break;

		case ENABLE:
			bkpt->enabled = true;
			breakpt_print();
			break;

		case DISABLE:
			bkpt->enabled = false;
			breakpt_print();
			break;
		};

		break;

	case RC_ERROR:
		USER("gdb reported error for command \"%s %s\"\n\t%s\n", argv[0], argv[1], resp->result->value->value);
		break;

	default:
		WARN("unhandled result class %d for \"%s %s\"\n", resp->rclass, argv[0], argv[1]);
		break;
	};

	gdb_result_free(resp->result);

	return 0;
}

void cmd_break_help(int argc, char** argv){
	unsigned int i;
	const struct user_subcmd_t* scmd;


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
			scmd = user_subcmd::lookup(argv[i], strlen(argv[i]));

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
void breakpt_read(gdb_result_t* result, breakpt_t* bkpt){
	gdb_result_t* r;


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
			if(r->value->type == VT_RESULT_LIST)
				breakpt_read((gdb_result_t*)r->value->value, bkpt);
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

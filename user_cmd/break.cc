#include <common/log.h>
#include <common/list.h>
#include <gdb/gdb.h>
#include <gui/gui.h>
#include <gdb/breakpoint.h>
#include <user_cmd/cmd.h>
#include <user_cmd/subcmd.hash.h>
#include <string.h>
#include <stdlib.h>
#include <map>
#include <string>


using namespace std;


/* static variables */
static map<string, gdb_breakpoint_t*> breakpt_lst;

static const char* cmd_str[] = {
	"break-insert",
	"break-delete",
	"break-enable",
	"break-disable",
};


/* static prototypes */
void breakpt_print();


/* global functions */
int cmd_break_exec(gdbif* gdb, int argc, char** argv){
	char key[256];
	const struct user_subcmd_t* scmd;
	map<string, gdb_breakpoint_t*>::iterator it;
	gdb_breakpoint_t* bkpt;


	if(argc != 3){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_break_help(1, argv);
		return 0;
	}

	bkpt = 0;
	scmd = user_subcmd::lookup(argv[1], strlen(argv[1]));

	if(scmd == 0){
		USER("invalid sub-command \"%s\" to command \"%s\"\n", argv[1], argv[0]);
		return 0;
	}

	switch(scmd->id){
	case ADD:
		if(gdb->mi_issue_cmd((char*)"break-insert", RC_DONE, result_to_brkpt, (void**)&bkpt, "%s", argv[2]) == 0){
			if(bkpt->filename != 0)	snprintf(key, 256, "%s:%d", bkpt->filename, bkpt->line);
			else					snprintf(key, 256, "%s", bkpt->at);

			breakpt_lst[key] = bkpt;
			breakpt_print();
			ui->win_anno_add(ui->win_getid(bkpt->fullname), bkpt->line, "b", "Black", "DarkRed");

			USER("added break-point \"%s\"\n", key);
		}
		else
			USER("error adding break-point\n");

		break;
	
	case DELETE:
	case ENABLE:
	case DISABLE:
		it = breakpt_lst.find(argv[2]);

		if(it == breakpt_lst.end()){
			USER("error breakpoint \"%s\" not found\n", argv[2]);
			return 0;
		}

		bkpt = it->second;

		switch(scmd->id){
		case DELETE:
			if(gdb->mi_issue_cmd((char*)"break-delete", RC_DONE, 0, 0, "%d", it->second->num) == 0){
				USER("deleted break-point \"%s\"\n", it->first.c_str());
				ui->win_anno_delete(ui->win_getid(bkpt->fullname), bkpt->line, "b");

				delete it->second;
				breakpt_lst.erase(it);
				breakpt_print();
			}
			else
				USER("error deleting break-point \"%s\" - \"%s %s\"\n", it->first.c_str());

			break;

		case ENABLE:
			if(gdb->mi_issue_cmd((char*)"break-enable", RC_DONE, 0, 0, "%d", it->second->num) == 0){
				USER("enabled break-point \"%s\"\n", it->first.c_str());

				bkpt->enabled = true;
				breakpt_print();
				ui->win_anno_add(ui->win_getid(bkpt->fullname), bkpt->line, "b", "Black", "DarkRed");
			}
			else
				USER("error enabling break-point \"%s\" - \"%s %s\"\n", it->first.c_str());

			break;

		case DISABLE:
			if(gdb->mi_issue_cmd((char*)"break-disable", RC_DONE, 0, 0, "%d", it->second->num) == 0){
				USER("disabled break-point \"%s\"\n", it->first.c_str());

				bkpt->enabled = false;
				breakpt_print();
				ui->win_anno_add(ui->win_getid(bkpt->fullname), bkpt->line, "b", "Black", "Yellow");
			}
			else
				USER("error disabling break-point \"%s\" - \"%s %s\"\n", it->first.c_str());

			break;
		};

		break;

	default:
		USER("unhandled sub command \"%s\" to \"%s\"\n", argv[1], argv[0]);
		return 0;
	};

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
void breakpt_print(){
	int win_id_break;
	map<string, gdb_breakpoint_t*>::iterator it;
	gdb_breakpoint_t* bkpt;


	win_id_break = ui->win_getid("breakpoints");

	if(win_id_break < 0)
		return;

	// TODO use atomicStart for the loop
	ui->win_clear(win_id_break);

	for(it=breakpt_lst.begin(); it!=breakpt_lst.end(); it++){
		bkpt = it->second;

		if(bkpt->enabled){
			if(bkpt->filename != 0)	ui->win_print(win_id_break, "   %s:%d\n", bkpt->filename, bkpt->line);
			else					ui->win_print(win_id_break, "   %s\n", bkpt->at);
		}
		else{
			if(bkpt->filename != 0)	ui->win_print(win_id_break, "   %s:%d [disabled]\n", bkpt->filename, bkpt->line);
			else					ui->win_print(win_id_break, "   %s [disabled]\n", bkpt->at);
		}
	}
}

#include <common/defaults.h>
#include <common/log.h>
#include <common/list.h>
#include <common/map.h>
#include <common/dynarray.h>
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


/* static prototypes */
void breakpt_print(char *filename = 0);


/* global functions */
bool cmd_break_exec(int argc, char **argv){
	char key[256];
	const struct user_subcmd_t *scmd;
	FILE *fp;
	map<string, gdb_breakpoint_t*>::iterator it;
	gdb_breakpoint_t *bkpt;


	if(argc < 2){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_break_help(1, argv);
		return false;
	}

	scmd = user_subcmd::lookup(argv[1], strlen(argv[1]));

	if(scmd == 0){
		USER("invalid sub-command \"%s\" to command \"%s\"\n", argv[1], argv[0]);
		return false;
	}

	if((scmd->id == ADD || scmd->id == DELETE || scmd->id == ENABLE || scmd->id == DISABLE || scmd->id == COMPLETE || scmd->id == EXPORT) && argc < 3){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_var_help(2, argv);
		return false;
	}

	switch(scmd->id){
	case ADD:
		if(gdb->mi_issue_cmd("break-insert", (gdb_result_t**)&bkpt, "%ssq %d", argv + 2, argc - 2) != 0)
			return false;

		if(bkpt->filename != 0)	snprintf(key, 256, "%s:%d", bkpt->filename, bkpt->line);
		else					snprintf(key, 256, "%s", bkpt->at);

		// check if breakpoint already exists - gdb doesn't check this
		if(MAP_LOOKUP(breakpt_lst, key) == 0){
			breakpt_lst[key] = bkpt;
			breakpt_print();
			ui->win_anno_add(ui->win_create(bkpt->fullname), bkpt->line, "b", "Black", "DarkRed");

			USER("add break-point \"%s\"\n", key);
		}
		else{
			if(gdb->mi_issue_cmd("break-delete", 0, "%d", bkpt->num) != 0)
				return false;

			delete bkpt;
		}

		break;

	case DELETE:
	case ENABLE:
	case DISABLE:
		it = breakpt_lst.find(argv[2]);

		if(it == breakpt_lst.end()){
			USER("error breakpoint \"%s\" not found\n", argv[2]);
			return false;
		}

		bkpt = it->second;

		switch(scmd->id){
		case DELETE:
			if(gdb->mi_issue_cmd("break-delete", 0, "%d", bkpt->num) != 0)
				return false;

			USER("delete break-point \"%s\"\n", it->first.c_str());
			ui->win_anno_delete(ui->win_create(bkpt->fullname), bkpt->line, "b");

			delete bkpt;
			breakpt_lst.erase(it);
			breakpt_print();
			break;

		case ENABLE:
			if(gdb->mi_issue_cmd("break-enable", 0, "%d", bkpt->num) != 0)
				return false;

			USER("enable break-point \"%s\"\n", it->first.c_str());

			bkpt->enabled = true;
			breakpt_print();

			ui->win_anno_delete(ui->win_create(bkpt->fullname), bkpt->line, "b");
			ui->win_anno_add(ui->win_create(bkpt->fullname), bkpt->line, "b", "Black", "DarkRed");
			break;

		case DISABLE:
			if(gdb->mi_issue_cmd("break-disable", 0, "%d", bkpt->num) != 0)
				return false;

			USER("disable break-point \"%s\"\n", it->first.c_str());

			bkpt->enabled = false;
			breakpt_print();

			ui->win_anno_delete(ui->win_create(bkpt->fullname), bkpt->line, "b");
			ui->win_anno_add(ui->win_create(bkpt->fullname), bkpt->line, "b", "Black", "Yellow");
			break;

		default:
			break;
		};

		break;

	case COMPLETE:
		breakpt_print(argv[2]);
		break;

	case EXPORT:
		fp = fopen(argv[2], "w");

		if(fp == 0){
			DEBUG("unable to open file \"%s\"\n", argv[2]);
			return false;
		}

		for(it=breakpt_lst.begin(); it!=breakpt_lst.end(); it++){
			bkpt = it->second;

			fprintf(fp, "Break add ");

			if(bkpt->condition)		fprintf(fp, "-c %s ", bkpt->condition);
			if(bkpt->ignore_cnt)	fprintf(fp, "-i %s ", bkpt->ignore_cnt);
			if(bkpt->temporary)		fprintf(fp, "-t ");

			if(bkpt->filename)		fprintf(fp, "%s:%d", bkpt->filename, bkpt->line);
			else					fprintf(fp, "%s", bkpt->at);

			fprintf(fp, "\n");
		}

		fprintf(fp, "\n");
		fclose(fp);

		USER("export breakpoints to \"%s\"\n", argv[2]);
		break;

	case VIEW:
		breakpt_print();
		break;

	default:
		USER("unhandled sub command \"%s\" to \"%s\"\n", argv[1], argv[0]);
	};

	return false;
}

void cmd_break_cleanup(){
	map<string, gdb_breakpoint_t*>::iterator it;


	for(it=breakpt_lst.begin(); it!=breakpt_lst.end(); it++)
		delete it->second;

	breakpt_lst.clear();
}

void cmd_break_help(int argc, char **argv){
	int i;
	const struct user_subcmd_t *scmd;


	ui->win_atomic(0, true);

	if(argc == 1){
		USER("usage %s <sub-command> <arg>\n", argv[0]);
		USER("   sub-commands:\n");
		USER("       add [opt] <location>     add breakpoint\n");
		USER("       delete <location>        delete breakpoint\n");
		USER("       enable <location>        enable breakpoint\n");
		USER("       disable <location>       disable breakpoint\n");
		USER("       view                     update breakpoint window\n");
		USER("       complete <file> <sync>   get list of breakpoints\n");
		USER("       export <file> <sync>     export breakpoints to vim script\n");
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
				USER("usage %s %s [-i <count> -c <condition>] <location>\n", argv[0], argv[i]);
				USER("   add new breakpoint, with <location> being any of\n");
				USER("      - function\n");
				USER("      - filename:linenum\n");
				USER("      - filename:function\n");
				USER("      - *address\n\n");
				USER("   options\n");
				USER("      -i <count>       set ignore count to <count>\n");
				USER("      -c <condition>   only break if <condition> is met\n");
				USER("      -t               add temporary breakpoint\n");
				USER("      -h               add hardware breakpoint\n");
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

			case COMPLETE:
				USER("usage %s %s <file> <sync>\n", argv[0], argv[i]);
				USER("          print '\\n' seprated list of breakpoint to file <file>, using file <sync> to sync with vim\n");
				USER("\n");
				break;

			case EXPORT:
				USER("usage %s %s <file> <sync>\n", argv[0], argv[1]);
				USER("         export breakpoints to vim script <file>, using file <sync> to sync with vim\n");
				USER("\n");
				break;

			case VIEW:
				break;

			default:
				USER("invalid sub-command \"%s\" to command \"%s\"\n", argv[i], argv[0]);
			};
		}
	}

	ui->win_cursor_set(ui->win_getid(USERLOG_NAME), -1);
	ui->win_atomic(0, false);
}


/* local functions */
void breakpt_print(char *filename){
	static dynarray obuf;
	int win_id;
	FILE *fp;
	map<string, gdb_breakpoint_t*>::iterator it;
	gdb_breakpoint_t *bkpt;


	if(filename){
		win_id = 0;
		fp = fopen(filename, "w");

		if(fp == 0)
			return;
	}
	else{
		fp = 0;
		win_id = ui->win_getid(BREAKPOINTS_NAME);

		if(win_id < 0)
			return;

		obuf.clear();
	}


	for(it=breakpt_lst.begin(); it!=breakpt_lst.end(); it++){
		bkpt = it->second;

		if(filename){
				if(bkpt->filename != 0)	fprintf(fp, "%s:%d\\n", bkpt->filename, bkpt->line);
				else					fprintf(fp, "%s\\n", bkpt->at);
		}
		else{
			if(bkpt->filename != 0)	obuf.add("%s:%d", bkpt->filename, bkpt->line);
			else					obuf.add("%s", bkpt->at);

			obuf.add("%s%s%s%s%s\n",
				(bkpt->enabled ? "" : " [disabled]"),
				(bkpt->ignore_cnt ? " after " : ""), (bkpt->ignore_cnt ? bkpt->ignore_cnt : ""),
				(bkpt->condition ? " if " : ""), (bkpt->condition ? bkpt->condition : "")
			);
		}
	}

	if(!filename){
		ui->win_atomic(win_id, true);

		ui->win_clear(win_id);
		ui->win_print(win_id, obuf.data());

		ui->win_atomic(win_id, false);
	}
	else
		fclose(fp);
}

#include <common/defaults.h>
#include <common/list.h>
#include <common/log.h>
#include <common/map.h>
#include <common/dynarray.h>
#include <gui/gui.h>
#include <gdb/gdb.h>
#include <gdb/result.h>
#include <gdb/frame.h>
#include <gdb/variable.h>
#include <user_cmd/cmd.h>
#include <user_cmd/subcmd.hash.h>
#include <string.h>


/* static variables */
static gdb_frame_t* callstack = 0;
static map<unsigned int, gdb_variable_t*> line_vars;
static map<unsigned int, gdb_frame_t*> line_frames;


/* global functions */
int cmd_callstack_exec(int argc, char** argv){
	gdb_variable_t* var;
	gdb_frame_t* frame;
	const struct user_subcmd_t* scmd;
	FILE* fp;
	map<unsigned int, gdb_frame_t*>::iterator it_frame;
	map<unsigned int, gdb_variable_t*>::iterator it_var;


	if(argc < 2){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_callstack_help(1, argv);
		return 0;
	}

	scmd = user_subcmd::lookup(argv[1], strlen(argv[1]));

	if(scmd == 0){
		USER("invalid sub-command \"%s\" to command \"%s\"\n", argv[1], argv[0]);
		return 0;
	}

	if(((scmd->id == FOLD || scmd->id == COMPLETE) && argc < 3) || ((scmd->id == SET || scmd->id == FORMAT) && argc < 4)){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_callstack_help(2, argv);
		return 0;
	}

	switch(scmd->id){
	case SET:
	case FORMAT:
		var = MAP_LOOKUP(line_vars, atoi(argv[2]));

		if(var == 0){
			USER("no variable at line \"%s\"\n", argv[2]);
			return 0;
		}
		break;

	default:
		var = 0;
		break;
	};

	switch(scmd->id){
	case FOLD:
		var = MAP_LOOKUP(line_vars, atoi(argv[2]));

		if(var == 0){
			frame = MAP_LOOKUP(line_frames, atoi(argv[2]));

			if(frame == 0){
				USER("no variable or frame at line \"%s\"\n", argv[2]);
				return 0;
			}
			else{
				frame->expanded = frame->expanded ? false : true;
			}
		}
		else{
			if(var->nchilds){
				var->init_childs();

				if(var->childs_visible)	var->childs_visible = false;
				else					var->childs_visible = true;
			}
			else if(var->parent)
				var->parent->childs_visible = false;
		}

		cmd_callstack_print();
		break;

	case SET:
		var->set(argc - 3, argv + 3);

		gdb->memory_update();
		break;

	case FORMAT:
		var->format(argv[3]);
		cmd_callstack_print();
		break;

	case COMPLETE:
		fp = fopen(argv[2], "w");

		if(fp == 0)
			return -1;

		for(it_frame=line_frames.begin(); it_frame!=line_frames.end(); it_frame++)
			fprintf(fp, "%d\\n", it_frame->first);

		for(it_var=line_vars.begin(); it_var!=line_vars.end(); it_var++)
			fprintf(fp, "%d\\n", it_var->first);

		fclose(fp);
		break;

	case VIEW:
		cmd_callstack_update();
		break;

	default:
		USER("unhandled sub command \"%s\" to \"%s\"\n", argv[1], argv[0]);
	};

	return 0;
}

void cmd_callstack_cleanup(){
	gdb_frame_t *frame;


	line_vars.clear();
	line_frames.clear();

	/* delete existing callstack */
	list_for_each(callstack, frame){
		list_rm(&callstack, frame);
		gdb_frame_t::release(frame);
	}
}

void cmd_callstack_help(int argc, char** argv){
	int i;
	const struct user_subcmd_t* scmd;


	ui->win_atomic(0, true);

	if(argc == 1){
		USER("usage: %s [sub-command] <args>...\n", argv[0]);
		USER("   sub-commands:\n");
		USER("      fold <line>              fold/unfold variable/frame\n");
		USER("      format <line> <fmt>      change variable output format\n");
		USER("      set <line> <value>       set variable\n");
		USER("      complete <file> <sync>   get list of variables/frames\n");
		USER("      view                     update callstack window\n");
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
			case FOLD:
				USER("usage %s %s <line>\n", argv[0], argv[i]);
				USER("   fold variable or frame at line <line>\n");
				USER("\n");
				break;

			case FORMAT:
				USER("usage %s %s <line> <format>\n", argv[0], argv[i]);
				USER("   change format of variable at line <line> to <format>\n");
				USER("   <format> = binary | decimal | hexadecimal | octal | natural\n");
				USER("\n");
				break;

			case SET:
				USER("usage %s %s <line> <value>\n", argv[0], argv[i]);
				USER("   set variable value at line <line>\n");
				USER("\n");
				break;

			case COMPLETE:
				USER("usage %s %s <file> <sync>\n", argv[0], argv[i]);
				USER("          print '\\n' seprated list of line numbers that contain variables or frames to file <file>, using file <sync> to sync with vim\n");
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

int cmd_callstack_update(){
	int win_id;
	gdb_frame_t *frame,
				*tframe,
				*framelst;
	gdb_variable_t *tvar, *var, *varlst;
	string ctx;


	win_id = ui->win_getid(CALLSTACK_NAME);

	if(win_id < 0)
		return 0;

	/* reset callstack */
	callstack = 0;

	/* get callstack frames */
	if(gdb->mi_issue_cmd("stack-list-frames", (gdb_result_t**)&framelst, "") != 0)
		return -1;

	/* add arguments and locals to each level of the callstack */
	list_for_each(framelst, tframe){
		// get list of arguments and locals for current frame
		if(gdb->mi_issue_cmd("stack-list-variables", (gdb_result_t**)&varlst, "--thread %u --frame %u 2", gdb->threadid(), tframe->level) != 0)
			return -1;

		// generate context string (context = '<function-name>:{<type><name>,}')
		ctx += tframe->function;
		ctx += ":";

		list_for_each(varlst, var){
			if(var->argument){
				ctx += var->type;
				ctx += var->name;
			}
		}

		// get frame and update callstack
		frame = gdb_frame_t::acquire(tframe->function, (char*)ctx.c_str(), tframe);

		list_add_tail(&callstack, frame);

		if(frame != tframe){
			list_rm(&framelst, tframe);
			gdb_frame_t::release(tframe);

			continue;
		}

		// update frame locals and arguments
		list_for_each(varlst, tvar){
			var = gdb_variable_t::acquire(tvar->name, O_CALLSTACK, (char*)ctx.c_str(), frame->level);
	
			if(var == 0)
				return -1;

			// increment the reference counter of new variables to avoid
			// variable from being delete once deleting the frame
			if(var->refcnt == 1)
				var->refcnt++;

			var->argument = tvar->argument;

			if(var->argument)	frame->args.push_back(var);
			else				frame->locals.push_back(var);

			list_rm(&varlst, tvar);
			
			if(gdb_variable_t::release(tvar) != 0)
				return -1;
		}
	}

	/* update UI */
	cmd_callstack_print();

	return 0;
}

int cmd_callstack_print(){
	static dynarray obuf;
	unsigned int line;
	int win_id;
	gdb_frame_t* frame;
	list<gdb_variable_t*>::iterator it;


	win_id = ui->win_getid(CALLSTACK_NAME);

	if(win_id < 0)
		return 0;


	obuf.clear();

	line_vars.clear();
	line_frames.clear();

	line = 1;

	/* iterate through callstack */
	for(frame=list_last(callstack); frame!=0; frame=frame->prev){
		// print function name
		if(!frame->args.empty())
			obuf.add("%s ", frame->expanded ? "[-]" : "[+]");

		obuf.add("´fl%s`fl:´ln%d`ln ´fu%s`fu(", frame->filename, frame->line, frame->function);
		line_frames[line] = frame;

		if(!frame->args.empty() && frame->expanded){
			obuf.add("\n");
			line++;
		}

		// print arguments
		for(it=frame->args.begin(); it!=frame->args.end(); it++){
			(*it)->print(&obuf, &line, &line_vars, frame->expanded, 1);

			if(!frame->expanded && (*it != frame->args.back()))
				obuf.add(",%s", (*it)->modified ? "" : " ");
		}

		obuf.add(")\n");
		line++;

		// print locals
		for(it=frame->locals.begin(); it!=frame->locals.end(); it++)
			(*it)->print(&obuf, &line, &line_vars, true, 1);

		obuf.add("\n");
		line++;

		if(frame == list_first(callstack))
			break;
	}

	ui->win_atomic(win_id, true);
	
	ui->win_clear(win_id);
	ui->win_print(win_id, "%s", obuf.data());

	ui->win_atomic(win_id, false);

	return 0;
}

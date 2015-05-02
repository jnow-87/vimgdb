#include <common/list.h>
#include <common/log.h>
#include <common/map.h>
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

	if(((scmd->id == FOLD || scmd->id == GET) && argc < 3) || ((scmd->id == SET || scmd->id == FORMAT) && argc < 4)){
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

		gdb_variable_t::get_changed();

		cmd_callstack_print();
		cmd_var_print();
		break;

	case FORMAT:
		var->format(argv[3]);
		cmd_callstack_print();
		break;

	case GET:
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

void cmd_callstack_help(int argc, char** argv){
	unsigned int i;
	const struct user_subcmd_t* scmd;


	ui->atomic(true);

	if(argc == 1){
		USER("usage: %s [sub-command] <args>...\n", argv[0]);
		USER("   sub-commands:\n");
		USER("      fold <line>          fold/unfold variable/frame\n");
		USER("      format <line> <fmt>  change variable output format\n");
		USER("      set <line> <value>   set variable\n");
		USER("      get <filename>       get list of variables/frames\n");
		USER("      view                 update callstack window\n");
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

			case GET:
				USER("usage %s %s <filename>\n", argv[0], argv[i]);
				USER("          print '\\n' seprated list of line numbers that contain variables or frames to file <filename>\n");
				USER("\n");
				break;

			case VIEW:
				break;

			default:
				USER("invalid sub-command \"%s\" to command \"%s\"\n", argv[i], argv[0]);
			};
		}
	}

	ui->atomic(false);
}

int cmd_callstack_update(){
	int win_id;
	gdb_result_t *result, *r;
	gdb_value_t *val;
	gdb_frame_t *frame;
	gdb_variable_t *vartmp, *var, *varlst;
	string ctx;


	varlst = 0;
	win_id = ui->win_getid("callstack");

	if(win_id < 0)
		return 0;

	/* delete existing callstack */
	list_for_each(callstack, frame){
		list_rm(&callstack, frame);
		delete frame;
	}

	/* get callstack frames */
	if(gdb->mi_issue_cmd((char*)"stack-list-frames", RC_DONE, 0, (void**)&result, "") != 0)
		return -1;

	if(result->var_id != IDV_STACK)
		goto err;

	/* parse callstack, adding frames to callstack */
	list_for_each((gdb_result_t*)result->value->value, r){
		switch(r->var_id){
		case IDV_FRAME:
			frame = 0;
			gdb_frame_t::result_to_frame((gdb_result_t*)r->value->value, &frame);

			list_add_head(&callstack, frame);
			break;
		};
	}

	gdb_result_free(result);

	/* add arguments and locals to each level of the callstack */
	list_for_each(callstack, frame){
		// get list of arguments and locals for current frame
		result = 0;

		if(gdb->mi_issue_cmd((char*)"stack-list-variables", RC_DONE, 0, (void**)&result, "--thread %u --frame %u 2", gdb->threadid(), frame->level) != 0)
			return -1;

		if(result->var_id != IDV_VARIABLES)
			goto err;

		// parse argument and locals names and update context string (context = '<function-name>:{<type><name>,}')
		ctx += frame->function;
		ctx += ":";

		list_for_each((gdb_value_t*)result->value->value, val){
			vartmp = 0;
			gdb_variable_t::result_to_variable((gdb_result_t*)val->value, (void**)&vartmp);
			
			list_add_tail(&varlst, vartmp);

			if(vartmp->argument){
				ctx += vartmp->type;
				ctx += vartmp->name;
			}
		}

		gdb_result_free(result);

		// update frame
		list_for_each(varlst, vartmp){
			var = gdb_variable_t::acquire(vartmp->name, O_CALLSTACK, (char*)ctx.c_str(), frame->level);
	
			if(var != 0){
				var->refcnt++;	// avoid variable being deleted once deleting the frame
				var->argument = vartmp->argument;

				list_add_tail((var->argument == true ? &frame->args : &frame->locals), var);
			}

			list_rm(&varlst, vartmp);
			gdb_variable_t::release(vartmp);
		}
	}

	/* update UI */
	cmd_callstack_print();

	return 0;

err:
	gdb_result_free(result);
	return -1;
}

int cmd_callstack_print(){
	unsigned int line;
	int win_id;
	gdb_frame_t* frame;
	gdb_variable_t* var;


	win_id = ui->win_getid("callstack");

	if(win_id < 0)
		return 0;

	ui->atomic(true);
	ui->win_clear(win_id);

	line_vars.clear();
	line_frames.clear();

	line = 1;

	/* iterate through callstack */
	for(frame=list_last(callstack); frame!=0; frame=frame->prev){
		// print function name
		if(frame->args != 0)
			ui->win_print(win_id, "%s ", frame->expanded ? "[-]" : "[+]");

		ui->win_print(win_id, "%s(", frame->function);
		line_frames[line] = frame;

		if(frame->args != 0 && frame->expanded){
			ui->win_print(win_id, "\n");
			line++;
		}

		// print arguments
		list_for_each(frame->args, var){
			var->print(win_id, &line, &line_vars, frame->expanded, 1);

			if(!frame->expanded && var != list_last(frame->args))
				ui->win_print(win_id, ", ");
		}

		ui->win_print(win_id, ")\n");
		line++;

		if(frame->expanded){
			ui->win_print(win_id, "\n");
			line++;
		}

		// print locals
		list_for_each(frame->locals, var){
			var->print(win_id, &line, &line_vars, true, 1);
		}

		ui->win_print(win_id, "\n");
		line++;

		if(frame == list_first(callstack))
			break;
	}

	ui->atomic(false);
}

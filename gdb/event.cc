#include <common/log.h>
#include <common/file.h>
#include <common/list.h>
#include <gdb/event.h>
#include <gdb/frame.h>
#include <gui/gui.h>
#include <string.h>


int evt_running(gdbif* gdb, gdb_result_t* result){
	gdb->running(true);
	return 0;
}

int evt_stopped(gdbif* gdb, gdb_result_t* result){
	gdb_result_t* r;
	gdb_frame_t* frame;
	char* reason;


	reason = 0;
	frame = 0;

	gdb->running(false);

	list_for_each(result, r){
		switch(r->var_id){
		case V_REASON:
			reason = (char*)r->value->value;
			break;

		case V_FRAME:
			conv_frame((gdb_result_t*)r->value->value, &frame);
			break;

		default:
			break;
		};
	}

	if(reason == 0)
		goto err;

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

	delete frame;
	return 0;

err:
	delete frame;
	return -1;
}

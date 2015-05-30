#include <common/defaults.h>
#include <common/list.h>
#include <common/map.h>
#include <common/log.h>
#include <common/string.h>
#include <gui/gui.h>
#include <gdb/gdb.h>
#include <gdb/result.h>
#include <gdb/memory.h>
#include <gdb/variable.h>
#include <user_cmd/cmd.h>
#include <user_cmd/subcmd.hash.h>
#include <map>


using namespace std;


/* macros */
#define CTOI(c) (unsigned int)((c) - ((c) >= 'a' ? 87 : 48))
#define ALIGN(val, base) ((val) & (~(base - 1)))


/* static variables */
static gdb_memory_t* mem_lst = 0;
static map<unsigned int, gdb_memory_t*> line_map;


/* global functions */
int cmd_memory_exec(int argc, char** argv){
	const struct user_subcmd_t* scmd;
	FILE* fp;
	gdb_memory_t* mem;
	map<unsigned int, gdb_memory_t*>::iterator it;


	if(argc < 2){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_memory_help(1, argv);
		return 0;
	}

	scmd = user_subcmd::lookup(argv[1], strlen(argv[1]));

	if(scmd == 0){
		USER("invalid sub-command \"%s\" to command \"%s\"\n", argv[1], argv[0]);
		return 0;
	}

	if(((scmd->id == DELETE || scmd->id == FOLD) && argc < 3) || ((scmd->id == ADD || scmd->id == SET || scmd->id == COMPLETE) && argc < 4)){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_memory_help(2, argv);
		return 0;
	}

	switch(scmd->id){
	case ADD:
		if(gdb->mi_issue_cmd("data-read-memory-bytes", (gdb_result_t**)&mem, "%ss %d", argv + 2, argc - 2) != 0)
			return -1;

		list_add_tail(&mem_lst, mem);

		USER("add memory dump at \"%s\" of size %u\n", mem->begin, mem->length);
		cmd_memory_update();
		break;
	
	case DELETE:
		mem = MAP_LOOKUP(line_map, atoi(argv[2]));

		if(mem == 0){
			USER("no memory segment at line %s\n", argv[2]);
			return 0;
		}

		list_rm(&mem_lst, mem);
		delete mem;

		USER("delete memory segment\n");

		cmd_memory_update();
		break;

	case SET:
		if(gdb->mi_issue_cmd("data-write-memory-bytes", 0, "%ss %d", argv + 2, argc - 2) != 0)
			return -1;
		
		gdb_variable_t::get_changed();

		cmd_memory_update();
		cmd_var_print();
		cmd_callstack_print();
		break;

	case FOLD:
		mem = MAP_LOOKUP(line_map, atoi(argv[2]));

		if(mem == 0){
			USER("no memory segment at line %s\n", argv[2]);
			return 0;
		}

		mem->expanded = mem->expanded ? false : true;

		cmd_memory_update();
		break;

	case COMPLETE:
		fp = fopen(argv[2], "w");

		if(fp == 0)
			return -1;

		for(it=line_map.begin(); it!=line_map.end(); it++)
			fprintf(fp, "%d\\n", it->first);

		fprintf(fp, "<addr>");

		list_for_each(mem_lst, mem)
			fprintf(fp, "%s\\n", mem->begin);

		fclose(fp);

		/* signal data availability */
		fp = fopen(argv[3], "w");
		fprintf(fp, "1\n");
		fclose(fp);
		break;

	case VIEW:
		cmd_memory_update();
		break;

	default:
		USER("unhandled sub command \"%s\" to \"%s\"\n", argv[1], argv[0]);
	};

	return 0;

}

void cmd_memory_help(int argc, char** argv){
	int i;
	const struct user_subcmd_t* scmd;


	ui->atomic(true);

	if(argc == 1){
		USER("usage: %s [sub-command] <args>...\n", argv[0]);
		USER("   sub-commands:\n");
		USER("      add <addr> <bytes>           add memory segment\n");
		USER("      delete <line>               delete memory segment\n");
		USER("      fold <line>                 fold/unfold memory segment\n");
		USER("      set <addr> <value> [<cnt>]  set memory\n");
		USER("      complete <filename>         get list of memory segments and addresses\n");
		USER("      view                        update memory window\n");
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
				USER("usage %s %s <addr> <bytes>\n", argv[0], argv[i]);
				USER("   add memory segment of size <bytes> starting at address <addr>\n");
				USER("\n");
				break;

			case DELETE:
				USER("usage %s %s <line>\n", argv[0], argv[i]);
				USER("   delete memory segment at line <line>\n");
				USER("\n");
				break;

			case FOLD:
				USER("usage %s %s <line>\n", argv[0], argv[i]);
				USER("   fold memory segment at line <line>\n");
				USER("\n");
				break;

			case SET:
				USER("usage %s %s <addr> <value> [<count>]\n", argv[0], argv[i]);
				USER("   set memory at address <addr> to <value>\n");
				USER("   optionally define the number of bytes to write as <count>, if count is greater than the content length <value> will be written repeatedly\n");
				USER("\n");
				break;

			case COMPLETE:
				USER("usage %s %s <filename>\n", argv[0], argv[i]);
				USER("   print list of line numbers and addresses to file <filename>\n");
				USER("   both lists are separated by '<addr>'\n");
				USER("   the items of each list are '\\n' separated\n");
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
	ui->atomic(false);
}

int cmd_memory_update(){
	static unsigned int len = 17;	// 8 byte + 0-byte + 8 byte for potential escape chars
	static char* ascii = new char[len];
	int win_id;
	char c;
	unsigned int i, j, line;
	long long addr, displ;
	gdb_memory_t *mem, *tmem;


	win_id = ui->win_getid(MEMORY_NAME);

	if(win_id < 0)
		return 0;

	line_map.clear();
	line = 1;

	ui->atomic(true);
	ui->win_clear(win_id);

	list_for_each(mem_lst, mem){
		addr = strtoll(mem->begin, 0, 16);

		/* get memory content */
		if(gdb->mi_issue_cmd("data-read-memory-bytes", (gdb_result_t**)&tmem, "%s %u", mem->begin, mem->length) != 0)
			break;

		/* print header */
		ui->win_print(win_id, "[%c] memory dump: %#0*p (%u bytes)\n", (mem->expanded ? '-' : '+'), sizeof(void*) * 2 + 2, addr, mem->length);
		line_map[line++] = mem;

		if(!mem->expanded){
			ui->win_print(win_id, "\n");
			line++;
			continue;
		}

		/* print preceding bytes, that are not part of content, to align the output to 8 bytes per line */
		j = 0;
		displ = ALIGN(addr, 8);

		ui->win_print(win_id, " %#0*p    ", sizeof(void*) * 2 + 2, displ);

		for(; displ<addr; displ++){
			ui->win_print(win_id, " ??");
			ascii[j++] = ' ';
		}

		/* print actual memory content */
		for(i=0; i<mem->length; i++, addr++){
			ui->win_print(win_id, " %s%2.2s", (memcmp(tmem->content + i * 2, mem->content + i * 2, 2) == 0 ? "" : "`"), tmem->content + i * 2);

			// update ascii string
			c = (char)(CTOI(tmem->content[i * 2]) * 16 + CTOI(tmem->content[i * 2 + 1]));
			ascii[j++] = c == '\0' ? ' ' : c;

			// print ascii string once reaching 8 bytes boundary
			if(addr + 1 == ALIGN(addr + 8, 8)){
				ascii[j] = 0;
				j = 0;
				ui->win_print(win_id, "    %s\n", strescape(ascii, &ascii, &len));
				line_map[line++] = mem;

				if(i + 1 < mem->length)
					ui->win_print(win_id, " %#0*p    ", sizeof(void*) * 2 + 2, 10 + 1);
			}
		}

		/* print trailing bytes, that are not part of content, to fill line */
		if(ALIGN(addr, 8) != addr){
			for(displ=ALIGN(addr + 8, 8); addr<displ; addr++){
				ui->win_print(win_id, " ??");
				ascii[j++] = ' ';
			}
		}

		ascii[j] = 0;
		ui->win_print(win_id, "    %s\n", strescape(ascii, &ascii, &len));
		line_map[line++] = mem;

		if(j != 0){
			ui->win_print(win_id, "\n");
			line++;
		}

		delete [] mem->content;
		mem->content = tmem->content;
		tmem->content = 0;

		delete tmem;
	}

	ui->atomic(false);

	return 0;
}

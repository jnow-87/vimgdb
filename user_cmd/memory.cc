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

	mem = 0;
	scmd = user_subcmd::lookup(argv[1], strlen(argv[1]));

	if(scmd == 0){
		USER("invalid sub-command \"%s\" to command \"%s\"\n", argv[1], argv[0]);
		return 0;
	}

	if(((scmd->id == DELETE || scmd->id == GET) && argc < 3) || ((scmd->id == ADD || scmd->id == SET) && argc < 4)){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_memory_help(2, argv);
		return 0;
	}

	switch(scmd->id){
	case ADD:
		if(gdb->mi_issue_cmd((char*)"data-read-memory-bytes", RC_DONE, gdb_memory_t::result_to_memory, (void**)&mem, "%ss %d", argv + 2, argc - 2) != 0)
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
		if(gdb->mi_issue_cmd((char*)"data-write-memory-bytes", RC_DONE, 0, 0, "%ss %d", argv + 2, argc - 2) != 0)
			return -1;
		
		gdb_variable_t::get_changed();

		cmd_memory_update();
		cmd_var_print();
		cmd_callstack_print();
		break;

	case GET:
		fp = fopen(argv[2], "w");

		if(fp == 0)
			return -1;

		for(it=line_map.begin(); it!=line_map.end(); it++)
			fprintf(fp, "%d\\n", it->first);

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
}

int cmd_memory_update(){
	static unsigned int len = 17;	// 8 byte + 0-byte + 8 byte for potential escape chars
	static char* ascii = new char[len];
	int win_id;
	char c;
	char* content_old;
	unsigned int i, j, line;
	long long addr, displ;
	gdb_memory_t* mem;


	win_id = ui->win_getid("memory");

	if(win_id < 0)
		return 0;

	line_map.clear();
	line = 1;

	ui->atomic(true);
	ui->win_clear(win_id);

	list_for_each(mem_lst, mem){
		addr = strtoll(mem->begin, 0, 16);

		// TODO handle update highlighting
		/* safe old content value */
		content_old = mem->content;
		mem->content = 0;

		/* get memory content */
		if(gdb->mi_issue_cmd((char*)"data-read-memory-bytes", RC_DONE, gdb_memory_t::result_to_memory, (void**)&mem, "%s %u", mem->begin, mem->length) != 0)
			break;

		/* print header */
		ui->win_print(win_id, "memory dump: %#0*p (%u bytes)\n", sizeof(void*) * 2 + 2, addr, mem->length);
		line_map[line++] = mem;

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
			ui->win_print(win_id, "%c%2.2s", (memcmp(content_old + i * 2, mem->content + i * 2, 2) == 0 ? ' ' : '`'), mem->content + i * 2);

			// update ascii string
			c = (char)(CTOI(mem->content[i * 2]) * 16 + CTOI(mem->content[i * 2 + 1]));
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
	}

	ui->atomic(false);

	return 0;
}

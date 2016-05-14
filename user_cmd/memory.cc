#include <common/defaults.h>
#include <common/list.h>
#include <common/map.h>
#include <common/log.h>
#include <common/string.h>
#include <common/dynarray.h>
#include <gui/gui.h>
#include <gdb/gdb.h>
#include <gdb/result.h>
#include <gdb/memory.h>
#include <gdb/variable.h>
#include <user_cmd/cmd.h>
#include <user_cmd/subcmd.hash.h>
#include <map>
#include <math.h>


using namespace std;


/* macros */
#define CTOI(c) (unsigned int)((c) - ((c) >= 'a' ? 87 : 48))
#define ALIGN(val, base) ((val) & (~((decltype(val))base - 1)))
#define PWR2(val) ({ \
	int li; \
	float lf; \
	\
	\
	lf = log2(val); \
	li = lf; \
	\
	(lf - li != 0.0 ? false : true); \
})


/* static variables */
static gdb_memory_t *mem_lst = 0;
static map<unsigned int, gdb_memory_t*> line_map;
static dynarray obuf;
static unsigned int asciib_len = 17;	// 8 byte + 0-byte + 8 byte for potential escape char
static char *asciib = new char[asciib_len];


/* global functions */
int cmd_memory_exec(int argc, char **argv){
	const struct user_subcmd_t *scmd;
	FILE *fp;
	gdb_memory_t *mem;
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

	if(((scmd->id == DELETE || scmd->id == FOLD || scmd->id == COMPLETE) && argc < 3) || ((scmd->id == ADD || scmd->id == SET) && argc < 4)){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_memory_help(2, argv);
		return 0;
	}

	switch(scmd->id){
	case ADD:
		mem = gdb_memory_t::acquire(argv[2], (unsigned int)strtol(argv[3], 0, 0));

		if(mem == 0)
			return -1;

		USER("%d: %s %s %s %s\n", argc, argv[0], argv[1], argv[2], argv[3]);

		if(argc >= 5){
			USER("has alignment: %s\n", argv[4]);
			mem->alignment = strtol(argv[4], 0, 0);

			// check if alignment is power of 2
			if(!PWR2(mem->alignment)){
				USER("alignment for memory segment is no power of 2, falling back to default alignment 8\n");
				mem->alignment = 8;
			}

			// check if ascii buffer needs to be enlarged
			if(asciib_len < mem->alignment * 2 + 1){
				asciib_len = mem->alignment * 2 + 1;

				delete [] asciib;
				asciib = new char[asciib_len];
			}
		}

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
		if(gdb_memory_t::set((void*)strtoll(argv[2], 0, 0), argv[3], (argc > 4 ? atoi(argv[4]) : 0)) != 0)
			return -1;

		gdb->memory_update();
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
		break;

	case VIEW:
		cmd_memory_update();
		break;

	default:
		USER("unhandled sub command \"%s\" to \"%s\"\n", argv[1], argv[0]);
	};

	return 0;

}

void cmd_memory_cleanup(){
	gdb_memory_t *mem;


	line_map.clear();

	list_for_each(mem_lst, mem){
		list_rm(&mem_lst, mem);
		delete mem;
	}

	delete [] asciib;
}

void cmd_memory_help(int argc, char **argv){
	int i;
	const struct user_subcmd_t *scmd;


	ui->win_atomic(0, true);

	if(argc == 1){
		USER("usage: %s [sub-command] <args>...\n", argv[0]);
		USER("   sub-commands:\n");
		USER("      add <addr> <bytes>              add memory segment\n");
		USER("      delete <line>                   delete memory segment\n");
		USER("      fold <line>                     fold/unfold memory segment\n");
		USER("      set <addr> <value> [<nbytes>]   set memory\n");
		USER("      complete <file> <sync>          get list of memory segments and addresses\n");
		USER("      view                            update memory window\n");
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
				USER("usage %s %s <addr> <value> [<nbytes>]\n", argv[0], argv[i]);
				USER("   set memory at address <addr> to <value>\n");
				USER("   optionally define the number of bytes to write as <nbytes>, if count is greater than the content length <value> will be written repeatedly\n");
				USER("\n");
				break;

			case COMPLETE:
				USER("usage %s %s <file> <sync>\n", argv[0], argv[i]);
				USER("   print list of line numbers and addresses to file <file>, using file <sync> to sync with vim\n");
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
	ui->win_atomic(0, false);
}

int cmd_memory_update(){
	bool modified;
	int win_id;
	char c;
	unsigned int i, j, line;
	long long addr, displ;
	gdb_memory_t *mem;


	win_id = ui->win_getid(MEMORY_NAME);

	if(win_id < 0)
		return 0;

	line_map.clear();
	obuf.clear();
	line = 1;

	list_for_each(mem_lst, mem){
		addr = strtoll(mem->begin, 0, 16);

		/* get memory content */
		if(mem->update() != 0)
			return -1;

		/* print header */
		obuf.add("[%c] ´h0memory dump: %#0*x`h0 (%u bytes)\n", (mem->expanded ? '-' : '+'), sizeof(void*) * 2 + 2, addr, mem->length);
		line_map[line++] = mem;

		if(!mem->expanded){
			obuf.add("\n");
			line++;
			continue;
		}

		obuf.add(" %*s      ´h1", sizeof(void*) * 2, "");

		for(i=0; i<mem->alignment; i++)
			obuf.add("%2x", i);
		
		obuf.add("`h1\n");

		line_map[line++] = mem;

		/* print preceding bytes, that are not part of content, to align the output to mem->alignment bytes per line */
		j = 0;
		displ = ALIGN(addr, mem->alignment);

		obuf.add(" ´h1%#0*x`h1    ", sizeof(void*) * 2 + 2, displ);

		for(; displ<addr; displ++){
			obuf.add("??");
			asciib[j++] = ' ';
		}

		/* print actual memory content */
		for(i=0; i<mem->length; i++, addr++){
			modified = memcmp(mem->content + i * 2, mem->content_old + i * 2, 2);

			obuf.add("%s%2.2s%s", (modified ? "´c" : ""), mem->content + i * 2, (modified ? "`c" : ""));

			// update ascii string
			c = (char)(CTOI(mem->content[i * 2]) * 16 + CTOI(mem->content[i * 2 + 1]));
			asciib[j++] = c == '\0' ? ' ' : c;

			// print ascii string and next address once reaching the alignent boundary
			if(addr + 1 == ALIGN(addr + mem->alignment, mem->alignment)){
				asciib[j] = 0;
				j = 0;
				obuf.add("    ´h2%s`h2\n", strescape(asciib, &asciib, &asciib_len));
				line_map[line++] = mem;

				if(i + 1 < mem->length)
					obuf.add(" ´h1%#0*x`h1    ", sizeof(void*) * 2 + 2, addr + 1);
			}
		}

		/* print trailing bytes, that are not part of content, to fill line */
		if(ALIGN(addr, mem->alignment) != addr){
			for(displ=ALIGN(addr + mem->alignment, mem->alignment); addr<displ; addr++){
				obuf.add("??");
				asciib[j++] = ' ';
			}
		}

		asciib[j] = 0;
		obuf.add("    ´h2%s`h2\n", strescape(asciib, &asciib, &asciib_len));
		line_map[line++] = mem;

		if(j != 0){
			obuf.add("\n");
			line++;
		}
	}

	/* update ui */
	ui->win_atomic(win_id, true);

	ui->win_clear(win_id);
	ui->win_print(win_id, obuf.data());

	ui->win_atomic(win_id, false);

	return 0;
}

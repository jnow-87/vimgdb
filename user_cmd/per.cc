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
#include <user_cmd/pertypes.h>
#include <user_cmd/per.tab.h>
#include <user_cmd/per.lex.h>
#include <map>
#include <string>


using namespace std;


/* macros */
#define CTOI(c) (unsigned int)((c) - ((c) >= 'a' ? 87 : 48))
#define ALIGN(val, base) ((val) & (~(base - 1)))


/* static variables */
static char* per_file = 0;
static per_range_t* range_lst = 0;
static map<unsigned int, per_range_t*> line_map;
static map<string, per_register_t*> reg_map;


/* local prototypes */
static int per_cleanup();


/* global functions */
int cmd_per_exec(int argc, char** argv){
	int r;
	const struct user_subcmd_t* scmd;
	FILE* fp;
	per_range_t* range;
	per_register_t* reg;
	per_bits_t* bits;
	map<unsigned int, per_range_t*>::iterator it_range;
	map<string, per_register_t*>::iterator it_reg;


	if(argc < 2){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_per_help(1, argv);
		return 0;
	}

	scmd = user_subcmd::lookup(argv[1], strlen(argv[1]));

	if((scmd == 0 && argc < 2) || (scmd != 0 && ((scmd->id == FOLD && argc < 3) || ((scmd->id == SET || scmd->id == COMPLETE || scmd->id == EXPORT) && argc < 4)))){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_per_help(2, argv);
		return 0;
	}

	if(scmd == 0){
		/* de-initialise */
		per_cleanup();

		/* parser peripheral file */
		fp = fopen(argv[1], "r");

		if(fp == 0){
			USER("cannot open file \"%s\"\n", argv[1]);
			return -1;
		}

		r = perparse(fp, &range_lst);
		perlex_destroy();

		fclose(fp);
		
		if(r != 0){
			USER("error parsing peripheral file \"%s\"\n", argv[1]);
			return -1;
		}

		/* initialise memory segments */
		list_for_each(range_lst, range){
			// read respective memory
			if(gdb->mi_issue_cmd("data-read-memory-bytes", (gdb_result_t**)&(range->mem), "0x%x %u", range->base, range->size) != 0){
				USER("unable to create memory segment for range \"%s\" (%p, %u)\n", range->name, range->base, range->size);
				per_cleanup();
				return -1;
			}

			list_for_each(range->regs, reg){
				// check integrity of register to range
				if(reg->offset + reg->nbytes >  range->size){
					USER("error: offset/size of register \"%s\" exceeds size of range \"%s\"\n", reg->name, range->name);
					per_cleanup();
					return -1;
				}

				// check integrity of bits to register
				list_for_each(reg->bits, bits){
					if(bits->idx + bits->nbits > reg->nbytes * 8){
						USER("error: index/size of bits \"%s\" exceeds size of register \"%s\"\n", bits->name, reg->name);
						per_cleanup();
						return -1;
					}
				}

				// update list of registers
				reg->parent = range;
				reg_map[reg->name] = reg;
			}

			USER("add memory segment for range \"%s\" (%p, %u)\n", range->name, range->base, range->size);
		}

		delete [] per_file;
		per_file = stralloc(argv[1], strlen(argv[1]));

		cmd_per_update();
	}
	else{
		switch(scmd->id){
		case SET:
			reg = MAP_LOOKUP(reg_map, argv[2]);

			if(reg == 0){
				USER("unknown register name \"%s\"\n", argv[2]);
				return 0;
			}

			if(gdb->mi_issue_cmd("data-write-memory-bytes", 0, "0x%x %s", (unsigned long long)reg->parent->base + reg->offset, argv[3]) != 0)
				return -1;
			
			gdb_variable_t::get_changed();

			cmd_memory_update();
			cmd_var_print();
			cmd_callstack_print();
			cmd_per_update();
			break;

		case FOLD:
			range = MAP_LOOKUP(line_map, atoi(argv[2]));

			if(range == 0){
				USER("no peripheral segment at line %s\n", argv[2]);
				return 0;
			}

			range->expanded = range->expanded ? false : true;

			cmd_per_update();
			break;

		case COMPLETE:
			fp = fopen(argv[2], "w");

			if(fp == 0)
				return -1;

			for(it_range=line_map.begin(); it_range!=line_map.end(); it_range++)
				fprintf(fp, "%d\\n", it_range->first);

			fprintf(fp, "<regs>");

			for(it_reg=reg_map.begin(); it_reg!=reg_map.end(); it_reg++)
				fprintf(fp, "%s\\n", it_reg->second->name);

			fclose(fp);

			/* signal data availability */
			fp = fopen(argv[3], "w");

			if(fp == 0)
				return -1;

			fprintf(fp, "1\n");
			fclose(fp);
			break;

		case EXPORT:
			fp = fopen(argv[2], "w");

			if(fp == 0)
				return 0;

			if(per_file)		fprintf(fp, "Per %s\n\n", per_file);

			fclose(fp);

			USER("export peripheral commands to \"%s\"\n", argv[2]);

			/* signal data availability */
			fp = fopen(argv[3], "w");

			if(fp == 0)
				return -1;

			fprintf(fp, "1\n");
			fclose(fp);
			break;


		case VIEW:
			cmd_per_update();
			break;

		default:
			USER("unhandled sub command \"%s\" to \"%s\"\n", argv[1], argv[0]);
		};
	}

	return 0;
}

void cmd_per_help(int argc, char** argv){
	int i;
	const struct user_subcmd_t* scmd;


	ui->atomic(true);

	if(argc == 1){
		USER("usage: %s [sub-command] <args>...\n", argv[0]);
		USER("   sub-commands:\n");
		USER("      <file>                          set peripheral file\n");
		USER("      set <register> <value> [<cnt>]  set register\n");
		USER("      fold <line>                     fold/unfold peripheral segment\n");
		USER("      complete <file> <sync>          get list of peripheral segments and addresses\n");
		USER("      export <file> <sync>            export used peripheral file to vim script\n");
		USER("      view                            update per window\n");
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
			case SET:
				USER("usage %s %s <register> <value> [<count>]\n", argv[0], argv[i]);
				USER("   set register <register> to <value>\n");
				USER("\n");
				break;

			case FOLD:
				USER("usage %s %s <line>\n", argv[0], argv[i]);
				USER("   fold/unfold peripheral segment at line <line>\n");
				USER("\n");
				break;

			case COMPLETE:
				USER("usage %s %s <file> <sync>\n", argv[0], argv[i]);
				USER("   print list of line numbers and addresses to file <file>, using file <sync> to sync with vim\n");
				USER("   both lists are separated by '<addr>'\n");
				USER("   the items of each list are '\\n' separated\n");
				USER("\n");
				break;

			case EXPORT:
				USER("usage %s %s <file> <sync>\n", argv[0], argv[1]);
				USER("   export peripheral file to vim script <file>, using file <sync> to sync with vim\n");
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

int cmd_per_update(){
	char c;
	int win_id;
	unsigned int line;
	unsigned long long reg_val, bit_val;
	bool changed;
	gdb_memory_t *mem;
	per_range_t* range;
	per_register_t* reg;
	per_bits_t* bits;


	win_id = ui->win_getid(PER_NAME);

	if(win_id < 0)
		return 0;

	line_map.clear();
	line = 1;

	ui->atomic(true);
	ui->win_clear(win_id);

	list_for_each(range_lst, range){
		/* print header */
		ui->win_print(win_id, "[%c] %s\n", (range->expanded ? '-' : '+'), range->name);
		line_map[line++] = range;

		if(!range->expanded){
			ui->win_print(win_id, "\n");
			line++;
			continue;
		}

		/* get memory content */
		if(gdb->mi_issue_cmd("data-read-memory-bytes", (gdb_result_t**)&mem, "%s %u", range->mem->begin, range->mem->length) != 0){
			ui->atomic(false);
			return -1;
		}

		/* print register values */
		list_for_each(range->regs, reg){
			changed = memcmp(range->mem->content + reg->offset * 2, mem->content + reg->offset * 2, reg->nbytes * 2);

			ui->win_print(win_id, " %s = %s%.*s\n", reg->name, (changed ? "`" : ""), reg->nbytes * 2, mem->content + reg->offset * 2);

			line_map[line] = range;
			line++;

			// print bits
			if(reg->bits){
				c = mem->content[reg->offset * 2 + reg->nbytes * 2];
				mem->content[reg->offset * 2 + reg->nbytes * 2] = 0;

				reg_val = strtoll(mem->content + reg->offset * 2, 0, 16);
				mem->content[reg->offset * 2 + reg->nbytes * 2] = c;

				list_for_each(reg->bits, bits){
					bit_val = (reg_val & (bits->mask << bits->idx)) >> bits->idx;

					changed = (bit_val == bits->value) ? false : true;
					bits->value = bit_val;

					ui->win_print(win_id, "  %s %s%0*.*x", bits->name, (changed ? "`" : ""), bits->nbits / 4, bits->nbits / 4, bit_val);
				}

				line_map[line] = range;
				ui->win_print(win_id, "\n\n");
				line += 2;
			}
		}

		delete [] range->mem->content;
		range->mem->content = mem->content;
		mem->content = 0;
		delete mem;
	}

	ui->atomic(false);

	return 0;
}


/* local functions */
int per_cleanup(){
	per_range_t* range;


	reg_map.clear();

	list_for_each(range_lst, range){
		list_rm(&range_lst, range);
		delete range;
	}

	cmd_per_update();

	return 0;
}

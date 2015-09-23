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
static per_section_t* section_lst = 0;
static map<unsigned int, per_section_t*> line_map;
static map<string, per_register_t*> reg_map;


/* global functions */
int cmd_per_exec(int argc, char** argv){
	int r;
	const struct user_subcmd_t* scmd;
	FILE* fp;
	per_section_t* sec;
	per_range_t* range;
	per_register_t* reg;
	per_bits_t* bits;
	map<unsigned int, per_section_t*>::iterator it_sec;
	map<string, per_register_t*>::iterator it_reg;


	if(argc < 2){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_per_help(1, argv);
		return 0;
	}

	scmd = user_subcmd::lookup(argv[1], strlen(argv[1]));

	if((scmd == 0 && argc < 2) || (scmd != 0 && (((scmd->id == FOLD || scmd->id == COMPLETE || scmd->id == EXPORT) && argc < 3) || (scmd->id == SET && argc < 4)))){
		USER("invalid number of arguments to command \"%s\"\n", argv[0]);
		cmd_per_help(2, argv);
		return 0;
	}

	if(scmd == 0){
		/* de-initialise */
		cmd_per_cleanup();

		/* parser peripheral file */
		fp = fopen(argv[1], "r");

		if(fp == 0){
			USER("cannot open file \"%s\"\n", argv[1]);
			return -1;
		}

		r = perparse(fp, &section_lst);
		perlex_destroy();

		fclose(fp);
		
		if(r != 0){
			USER("error parsing peripheral file \"%s\"\n", argv[1]);
			return -1;
		}

		/* initialise memory segments */
		list_for_each(section_lst, sec){
			list_for_each(sec->ranges, range){
				// check if the range is a headline and not an actual range
				if(range->name)
					continue;

				// read respective memory
				range->mem = gdb_memory_t::acquire(range->base, range->size);

				if(range->mem == 0){
					USER("unable to create memory segment for section \"%s\" (%p, %u)\n", sec->name, range->base, range->size);

					cmd_per_cleanup();
					cmd_per_update();

					return -1;
				}

				list_for_each(range->regs, reg){
					if(reg->name)
						strdeescape(reg->name);

					// check if the register is a headline and not an actual register
					if(reg->nbytes == 0)
						continue;

					// check integrity of register to range
					if(reg->offset + reg->nbytes >  range->size){
						USER("error: offset/size of register \"%s\" exceeds size of range (%p)\n", reg->name, range->base);

						cmd_per_cleanup();
						cmd_per_update();

						return -1;
					}

					// check integrity of bits to register
					list_for_each(reg->bits, bits){
						if(bits->idx + bits->nbits > reg->nbytes * 8){
							USER("error: index/size of bits \"%s\" exceeds size of register \"%s\"\n", bits->name, reg->name);

							cmd_per_cleanup();
							cmd_per_update();
							
							return -1;
						}
					}

					// update list of registers
					reg->parent = range;

					if(MAP_LOOKUP(reg_map, reg->name)){
						USER("error: register \"%s\" defined twice in peripheral file \"%s\"\n", reg->name, argv[1]);

						cmd_per_cleanup();
						cmd_per_update();

						return -1;
					}

					reg_map[reg->name] = reg;
				}

				USER("add memory segment for section \"%s\" (%p, %u)\n", sec->name, range->base, range->size);
			}
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

			if(gdb_memory_t::set((void*)((unsigned long long)reg->parent->base + reg->offset), argv[3], reg->nbytes) != 0)
				return -1;

			gdb->memory_update();
			break;

		case FOLD:
			sec = MAP_LOOKUP(line_map, atoi(argv[2]));

			if(sec == 0){
				USER("no peripheral section at line %s\n", argv[2]);
				return 0;
			}

			sec->expanded = sec->expanded ? false : true;

			cmd_per_update();
			break;

		case COMPLETE:
			fp = fopen(argv[2], "w");

			if(fp == 0)
				return -1;

			for(it_sec=line_map.begin(); it_sec!=line_map.end(); it_sec++)
				fprintf(fp, "%d\\n", it_sec->first);

			fprintf(fp, "<regs>");

			for(it_reg=reg_map.begin(); it_reg!=reg_map.end(); it_reg++)
				fprintf(fp, "%s\\n", it_reg->second->name);

			fclose(fp);
			break;

		case EXPORT:
			fp = fopen(argv[2], "w");

			if(fp == 0)
				return 0;

			if(per_file)		fprintf(fp, "Per %s\n\n", per_file);

			fclose(fp);

			USER("export peripheral commands to \"%s\"\n", argv[2]);
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

void cmd_per_cleanup(){
	per_section_t* sec;


	delete [] per_file;
	per_file = 0;

	reg_map.clear();
	line_map.clear();

	list_for_each(section_lst, sec){
		list_rm(&section_lst, sec);
		delete sec;
	}
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
	unsigned int line, i;
	unsigned long long reg_val, bit_val;
	bool modified;
	gdb_memory_t *mem;
	per_section_t* sec;
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

	list_for_each(section_lst, sec){
		/* print header */
		ui->win_print(win_id, "[%c] ´h0%s`h0\n", (sec->expanded ? '-' : '+'), sec->name);
		line_map[line++] = sec;

		if(!sec->expanded){
			ui->win_print(win_id, "\n");
			line++;
			continue;
		}

		list_for_each(sec->ranges, range){
			/* print headline of range is not an actual range */
			if(range->name){
				ui->win_print(win_id, " ´h1%s`h1\n", range->name);
				line_map[line] = sec;
				line++;

				continue;
			}

			/* get memory content */
			mem = range->mem;

			if(mem->update() != 0){
				ui->atomic(false);
				return -1;
			}

			/* print register values */
			list_for_each(range->regs, reg){
				if(reg->nbytes == 0){
					ui->win_print(win_id, " ´h1%s%s%s`h1\n", (reg->name ? reg->name : ""), (reg->desc && reg->desc[0] ? " - " : ""), (reg->desc ? reg->desc : ""));
					line_map[line] = sec;
					line++;

					continue;
				}

				modified = memcmp(mem->content + reg->offset * 2, mem->content_old + reg->offset * 2, reg->nbytes * 2);

				ui->win_print(win_id, "  ´h2%s%s%s`h2 = %s%.*s%s\n", reg->name, (reg->desc && reg->desc[0] ? " - " : ""), (reg->desc ? reg->desc : ""), (modified ? "´c" : ""), reg->nbytes * 2, mem->content + reg->offset * 2, (modified ? "`c" : ""));

				line_map[line] = sec;
				line++;

				// print bits
				if(reg->bits){
					c = mem->content[reg->offset * 2 + reg->nbytes * 2];
					mem->content[reg->offset * 2 + reg->nbytes * 2] = 0;

					reg_val = strtoll(mem->content + reg->offset * 2, 0, 16);
					mem->content[reg->offset * 2 + reg->nbytes * 2] = c;

					i = 0;
					list_for_each(reg->bits, bits){
						if(i && i % 4 == 0){
							line_map[line] = sec;
							ui->win_print(win_id, "\n");
							line++;
						}

						bit_val = (reg_val & (bits->mask << bits->idx)) >> bits->idx;

						modified = (bit_val == bits->value) ? false : true;
						bits->value = bit_val;

						ui->win_print(win_id, "   ´h3%s`h3 %s%0*.*x%s", bits->name, (modified ? "´c" : ""), (bits->nbits + 3) / 4, (bits->nbits + 3) / 4, bit_val, (modified ? "`c" : ""));
						i++;
					}

					line_map[line] = sec;
					ui->win_print(win_id, "\n\n");
					line += 2;
				}
			}

			ui->win_print(win_id, "\n");
			line++;
		}
	}

	ui->atomic(false);

	return 0;
}

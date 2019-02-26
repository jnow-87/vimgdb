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
#include <user_cmd/pertypes.h>
#include <user_cmd/per.tab.h>
#include <user_cmd/per.lex.h>
#include <map>
#include <vector>
#include <string>


using namespace std;


/* macros */
#define CTOI(c) (unsigned int)((c) - ((c) >= 'a' ? 87 : 48))
#define ALIGN(val, base) ((val) & (~(base - 1)))
#define LOOKUP_LINE(_vec, _line)({ \
	auto it = (_vec).rbegin(); \
	\
	\
	for(; it!=(_vec).rend(); it++){ \
		if(it->line <= (_line)) \
			break; \
	} \
	\
	it != (_vec).rend() ? it->data : 0x0; \
})


/* types */
typedef struct{
	unsigned int line;
	void *data;
} line_map_t;


/* static variables */
static char *per_file = 0;
static per_range_t *range_lst = 0;
static vector<line_map_t> line_sec_map;
static vector<line_map_t> line_reg_map;


/* global functions */
int cmd_per_exec(int argc, char **argv){
	int r;
	const struct user_subcmd_t *scmd;
	FILE *fp;
	per_section_t *sec;
	per_range_t *range;
	per_register_t *reg;
	per_bits_t *bits;
	vector<line_map_t>::iterator line;


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
			range->mem = gdb_memory_t::acquire(range->base, range->size);

			if(range->mem == 0){
				USER("unable to create memory segment for section \"%s\" (%p, %u)\n", sec->name, range->base, range->size);

				cmd_per_cleanup();
				cmd_per_update();

				return -1;
			}

			list_for_each(range->sections, sec){
				list_for_each(sec->regs, reg){
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
				}

				USER("add peripheral section \"%s\"\n", sec->name);
			}
		}

		delete [] per_file;
		per_file = stralloc(argv[1], strlen(argv[1]));

		cmd_per_update();
	}
	else{
		switch(scmd->id){
		case SET:
			reg = (per_register_t*)LOOKUP_LINE(line_reg_map, (unsigned int)atoi(argv[2]));

			if(reg == 0){
				USER("no register at line %s\n", argv[2]);
				return 0;
			}

			if(reg->opt & REG_SWAP)
				strswap2(argv[3], strlen(argv[3]));

			if(gdb_memory_t::set((void*)((unsigned long long)reg->parent->base + reg->offset), argv[3], reg->nbytes) != 0)
				return -1;

			gdb->memory_update();
			break;

		case FOLD:
			sec = (per_section_t*)LOOKUP_LINE(line_sec_map, (unsigned int)atoi(argv[2]));

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

			for(line=line_sec_map.begin(); line!=line_sec_map.end(); line++)
				fprintf(fp, "%d\\n", line->line);

			fprintf(fp, "<regs>");

			for(line=line_reg_map.begin(); line!=line_reg_map.end(); line++)
				fprintf(fp, "%d\\n", line->line);

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
	per_range_t *range;


	delete [] per_file;
	per_file = 0;

	line_sec_map.clear();
	line_reg_map.clear();

	list_for_each(range_lst, range){
		list_rm(&range_lst, range);
		delete range;
	}
}

void cmd_per_help(int argc, char **argv){
	int i;
	const struct user_subcmd_t *scmd;


	ui->win_atomic(0, true);

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
	ui->win_atomic(0, false);
}

int cmd_per_update(){
	static dynarray obuf;
	char c;
	int win_id;
	unsigned int line, i;
	unsigned long int reg_val, bit_val;
	bool modified;
	gdb_memory_t *mem;
	per_section_t *sec;
	per_range_t *range;
	per_register_t *reg;
	per_bits_t *bits;


	win_id = ui->win_getid(PER_NAME);

	if(win_id < 0)
		return 0;

	line_sec_map.clear();
	line_reg_map.clear();
	obuf.clear();
	line = 1;

	/* generate output buffer */
	list_for_each(range_lst, range){
		// get memory content
		mem = range->mem;

		if(mem->update() != 0)
			return -1;

		list_for_each(range->sections, sec){
			// print header
			obuf.add("[%c] ´h0%s`h0\n", (sec->expanded ? '-' : '+'), sec->name);
			line_sec_map.push_back({.line = line, .data = sec});
			line++;

			if(!sec->expanded){
				obuf.add("\n");
				line++;
				continue;
			}

			// print register values
			list_for_each(sec->regs, reg){
				line_reg_map.push_back({.line = line, .data = reg});

				if(reg->nbytes == 0){
					obuf.add(" ´h1%s%s%s`h1\n", (reg->name ? reg->name : ""), (reg->desc && reg->desc[0] ? " - " : ""), (reg->desc ? reg->desc : ""));
					line++;

					continue;
				}

				modified = memcmp(mem->content + reg->offset * 2, mem->content_old + reg->offset * 2, reg->nbytes * 2);

				if(reg->opt & REG_SWAP)
					strswap2(mem->content + reg->offset * 2, reg->nbytes * 2);

				obuf.add("  ´h2%s%s%s`h2 = %s%.*s%s\n", reg->name, (reg->desc && reg->desc[0] ? " - " : ""), (reg->desc ? reg->desc : ""), (modified ? "´c" : ""), reg->nbytes * 2, mem->content + reg->offset * 2, (modified ? "`c" : ""));

				if(reg->opt & REG_SWAP)
					strswap2(mem->content + reg->offset * 2, reg->nbytes * 2);

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
							obuf.add("\n");
							line++;
						}

						bit_val = (reg_val & (bits->mask << bits->idx)) >> bits->idx;

						modified = (bit_val == bits->value) ? false : true;
						bits->value = bit_val;

						obuf.add("   ´h3%s`h3 %s%.*lx%s", bits->name, (modified ? "´c" : ""), (bits->nbits + 3) / 4, bit_val, (modified ? "`c" : ""));
						i++;
					}

					obuf.add("\n\n");
					line += 2;
				}
			}

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

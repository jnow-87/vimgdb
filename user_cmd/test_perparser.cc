#include <common/list.h>
#include <common/log.h>
#include <user_cmd/pertypes.h>
#include <user_cmd/per.lex.h>
#include <user_cmd/per.tab.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>


/* global functions */
int main(int argc, char** argv){
	FILE* fp;
	per_section_t *rlst, *sec;
	per_range_t *range;
	per_register_t* reg;
	per_bits_t* bits;


	/* init */
	if(argc < 2){
		USER("usage: %s <per-file>\n", argv[0]);
		return 0;
	}

	fp = fopen(argv[1], "r");

	if(fp == 0){
		USER("unable to open file \"%s\"\n", argv[1]);
		return 1;
	}

	if(log::init("/proc/self/fd/1", LOG_LEVEL) != 0)
		return 1;

	USER("parser return value: %d\n\n", perparse(fp, &rlst));
	perlex_destroy();

	fclose(fp);

	list_for_each(rlst, sec){
		USER("section: %s\n", sec->name);

		list_for_each(sec->ranges, range){
			if(!range->name){
				USER("range: 0x%lx %u\n", (unsigned long)range->base, range->size);

				list_for_each(range->regs, reg){
					if(reg->nbytes == 0){
						if(reg->name)	printf("    heading: %s\n", reg->name);
						else			printf("\n");

						continue;
					}

					USER("    reg: %s %u %u\n", reg->name, reg->offset, reg->nbytes);

					list_for_each(reg->bits, bits){
						USER("        bits: %s %u %#x\n", bits->name, bits->idx, bits->mask);
					}
				}

				USER("\n");
			}
			else
				USER("heading: %s\n", range->name);
		}
	}

	log::cleanup();

	return 0;
}

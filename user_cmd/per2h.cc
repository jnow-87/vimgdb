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


/* macros */
/**
 * \brief	remove leading and trailing spaces
 *
 * \param	s	string to be updated
 * \param	o	index of the first non-blank
 */
#define skip_blank(s, o) { \
	unsigned int i; \
	\
	\
	o = 0; \
	while(s[o] != 0 && s[o] == ' ') o++; \
	i = strlen(s) - 1; \
	while(i > 0 && s[i] == ' ') i--; \
	s[i + 1] = 0; \
}


/* global functions */
int main(int argc, char** argv){
	int retval = 1;
	unsigned int sec_off,
				 reg_off,
				 bit_off;
	FILE *per,
		 *header;
	per_section_t *rlst, *sec;
	per_range_t *range;
	per_register_t* reg;
	per_bits_t* bits;


	/* init */
	if(argc < 3){
		printf("usage: %s <per-file> <header-file>\n", argv[0]);
		goto e0;
	}

	if(log::init("/proc/self/fd/1", LOG_LEVEL) != 0)
		goto e0;

	per = fopen(argv[1], "r");

	if(per == 0){
		USER("error: unable to open file \"%s\" - %s\n", argv[1], strerror(errno));
		goto e1;
	}

	header = fopen(argv[2], "w");

	if(header == 0){
		USER("error: unable to open file \"%s\" - %s\n", argv[2], strerror(errno));
		goto e2;
	}

	/* parse peripheral file */
	if(perparse(per, &rlst) != 0){
		USER("error: parsing peripheral file failed\n");
		goto e3;
	}

	/* generate header */
	list_for_each(rlst, sec){
		skip_blank(sec->name, sec_off);
		fprintf(header, "/* %s */\n", sec->name + sec_off);

		list_for_each(sec->ranges, range){
			if(!range->name){
				list_for_each(range->regs, reg){
					if(reg->nbytes == 0)
						continue;

					skip_blank(reg->name, reg_off);

					fprintf(header,
						"// %s\n"
						"#define %s\t0x%lx\n\n"
						, reg->name + reg_off
						, reg->name + reg_off
						, (unsigned long int)range->base + reg->offset
					);

					list_for_each(reg->bits, bits){
						skip_blank(bits->name, bit_off);
						fprintf(header, "#define %s_%s\t%d\n", reg->name + reg_off, bits->name + bit_off, bits->idx);
					}
					
					if(!list_empty(reg->bits))
						fprintf(header, "\n");
				}
			}
		}
	}

	retval = 0;

	/* cleanup */
e3:
	perlex_destroy();

e2:
	fclose(per);

e1:
	log::cleanup();

e0:
	return retval;
}

#ifndef CONFIG_H
#define CONFIG_H


#include <common/log.h>


#define LOG_FILE	"/tmp/vimgdb.log"
#define LOG_LEVEL	(log_level_t)(INFO | WARN | ERROR | /*DEBUG | */ USER | TEST | TODO)
#define CMD_PROMPT	"cmd: "


#endif

#ifndef CONFIG_H
#define CONFIG_H


#include "log.h"


#define LOG_FILE	"/proc/self/fd/1"
#define LOG_LEVEL	(log_level_t)(INFO | WARN | ERROR)


#endif

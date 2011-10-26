#ifndef PCONSOLE_H_
#define PCONSOLE_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "defines.h"
#include "location.h"
#include "common/utils/commands.h"
#include "common/utils/sockets.h"
#include "common/utils/config.h"

#define SOCKET_UNIX_PATH "/tmp/lol"

void init_pconsole(void);
void *pconsole(void*);
void info(void *context, t_array *args);
void clean(void *context, t_array *args);
void trace(void *context, t_array *args);

#endif

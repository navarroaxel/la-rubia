#ifndef PCONSOLE_H_
#define PCONSOLE_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "common/utils/commands.h"

#define SOCKET_UNIX_PATH "/tmp/lol"

void pconsole(void);
void info(void *context, t_array *args);
void clean(void *context, t_array *args);
void trace(void *context, t_array *args);

#endif

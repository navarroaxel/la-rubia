#ifndef CONSOLE_H_
#define CONSOLE_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "common/utils/commands.h"

void console(void);
void info(void *context, t_array *args);
void clean(void *context, t_array *args);
void trace(void *context, t_array *args);

#endif

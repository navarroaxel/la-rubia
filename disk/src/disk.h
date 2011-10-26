#ifndef DISK_H_
#define DISK_H_

#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include "common/utils/sockets.h"
#include "common/collections/blist.h"
#include "common/nipc.h"
#include "common/utils/commands.h"

void info(void *context, t_array *args);
void clean(void *context, t_array *args);
void trace(void *context, t_array *args);

#endif

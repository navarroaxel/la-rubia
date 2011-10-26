#ifndef CONSOLE_H_
#define CONSOLE_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "defines.h"
#include "common/utils/sockets.h"
#include "common/utils/config.h"
#include "head.h"
#include "headHelper.h"

#define SOCKET_UNIX_PATH "/tmp/lol"

void init_console(t_blist *waiting);
void *console(void *waiting);
void enqueueOperation(t_blist *waiting, t_disk_operation *op);

#endif

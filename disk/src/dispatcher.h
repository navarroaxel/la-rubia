#ifndef DISPATCHER_H_
#define DISPATCHER_H_

#include <stdbool.h>
#include <pthread.h>
#include "disk.h"
#include "defines.h"
#include "common/utils/sockets.h"
#include "common/collections/blist.h"
#include "common/nipc.h"
#include "headtrace.h"

void init_dispatcher(t_blist *processed, t_log *log);
void *dispatcher(void *args);

#endif

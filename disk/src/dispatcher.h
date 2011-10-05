#ifndef DISPATCHER_H_
#define DISPATCHER_H_

#include <stdbool.h>
#include <pthread.h>
#include "disk.h"
#include "common/utils/sockets.h"
#include "common/collections/blist.h"
#include "common/nipc.h"

void init_dispatcher(t_blist *ready);
void *dispatcher(void *args);

#endif

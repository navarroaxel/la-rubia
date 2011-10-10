#ifndef DISK_H_
#define DISK_H_

#include <stdlib.h>
#include <stdbool.h>
#include "common/utils/sockets.h"
#include "common/collections/blist.h"
#include "common/nipc.h"
#include "raid.h"

void *disk(void *args);
int getoffset(t_nipc *nipc);

#endif

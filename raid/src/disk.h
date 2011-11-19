#ifndef DISK_H_
#define DISK_H_

#include <stdlib.h>
#include <stdbool.h>
#include "common/utils/sockets.h"
#include "common/collections/list.h"
#include "common/nipc.h"
#include "operations.h"
#include "disks.h"

void *disk(void *args);
void processReadRs(t_disk *dsk, t_nipc *nipc);
void processWriteRs(t_disk *dsk, t_nipc *nipc);
void reallocateoperations(t_disk *dsk);

void disk_increasepending(t_disk *dsk);
void disk_decreasepending(t_disk *dsk);

#endif

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
int getoffset(t_nipc *nipc);
void processReadRs(t_disk *dsk, t_nipc *nipc);
void processWriteRs(t_disk *dsk, t_nipc *nipc);

#endif

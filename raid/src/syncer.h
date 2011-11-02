#ifndef SYNCER_H_
#define SYNCER_H_

#include <fcntl.h>
#include <pthread.h>
#include "disks.h"

extern uint32_t raidoffsetlimit;

void init_syncer(t_disk *dsk);
void *syncer(void *args);
void enqueueread(int offset, t_blist *syncqueue);
void enqueuewrite(t_disk *dsk, t_disk_readSectorRs *rs);

#endif

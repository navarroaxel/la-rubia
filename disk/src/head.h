#ifndef HEAD_H_
#define HEAD_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "common/collections/blist.h"
#include "common/collections/list.h"
#include "common/utils/config.h"
#include "common/utils/log.h"
#include "disk.h"
#include "location.h"
#include "headtrace.h"

struct queues {
	t_blist *waiting;
	t_blist *processed;
	t_log *log;
};

void init_head(t_blist *waiting, t_blist *processed, t_log *log);
void *head_cscan(void *args);
void *head_fscan(void *args);
t_location *head_currentlocation(void);
t_headtrace *head_cscanmove(uint32_t requested);
t_headtrace *head_fscanmove(uint32_t requested, bool asc);

void init_disk();
int disk_read(t_location *location, t_sector *sector);
int disk_write(t_location *location, t_sector *sector);

#endif

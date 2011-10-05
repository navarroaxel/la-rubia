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
#include "disk.h"

void init_head(t_blist *operations, t_blist *ready);
void *head_cscan(void *args);
void *head_fscan(void *args);
void init_disk();
int disk_read(uint32_t offset, t_sector *sector);
int disk_write(uint32_t offset, t_sector *sector);

#endif

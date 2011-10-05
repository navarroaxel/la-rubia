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
#include "disk.h"

void init_head(t_blist *operations, t_blist *ready);
void *head(void *args);
void init_disk();
void disk_read(uint32_t offset, t_sector *sector);
void disk_write(uint32_t offset, t_sector *sector);

#endif

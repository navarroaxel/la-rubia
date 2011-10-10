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

typedef struct location {
	uint16_t cylinder;
	uint16_t sector;
} location;

struct trace {
	location current;
	location requested;
	t_list *path;
	uint32_t time;
	location next;
};

void init_head(t_blist *waiting, t_blist *processed);
void *head_cscan(void *args);
void *head_fscan(void *args);

location *location_clone(location *l);
int islimitcylinder(int cylinder);
int islimitsector(int sector);
int getcylinder(uint32_t offset);

void init_disk();
int disk_read(uint32_t offset, t_sector *sector);
int disk_write(uint32_t offset, t_sector *sector);

#endif

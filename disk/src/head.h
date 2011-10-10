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
#include "common/utils/log.h"
#include "disk.h"

typedef struct t_location {
	uint16_t cylinder;
	uint16_t sector;
} t_location;

typedef struct t_planning {
	t_location current;
	t_location requested;
	t_list *path;
	uint32_t time;
	t_location next;
} t_planning;

struct queues {
	t_blist *waiting;
	t_blist *processed;
	t_log *logFile;
};

void init_head(t_blist *waiting, t_blist *processed, t_log *logFile);
void *head_cscan(void *args);
void *head_fscan(void *args);
t_planning *head_cscanmove(uint32_t requested);

int islimitcylinder(int cylinder);
int islimitsector(int sector);
uint16_t getcylinder(uint32_t offset);
uint16_t getsector(uint32_t offset);

t_planning *planning_create();
void planning_destroy(t_planning *p);

t_location *location_create(uint32_t offset);
t_location *location_clone(t_location *l);
void location_set(t_location *l, uint32_t offset);
void location_copy(t_location *l1, t_location *l2);
void location_destroy(t_location *l);

void init_disk();
int disk_read(uint32_t offset, t_sector *sector);
int disk_write(uint32_t offset, t_sector *sector);

#endif

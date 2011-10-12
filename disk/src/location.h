#ifndef LOCATION_H_
#define LOCATION_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "common/collections/list.h"
#include "headHelper.h"

typedef struct t_location {
	uint16_t cylinder;
	uint16_t sector;
} t_location;

t_location *location_create(uint32_t offset);
t_location *location_clone(t_location *l);
void location_set(t_location *l, uint32_t offset);
void location_copy(t_location *l1, t_location *l2);
int location_string(t_location *l, char *s);
int locations_string(t_list *locations, char *s);
void location_destroy(t_location *l);

#endif

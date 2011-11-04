#ifndef LOCATION_H_
#define LOCATION_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "common/collections/list.h"
#include "defines.h"
#include "headHelper.h"

t_location *location_create(uint32_t offset);
t_location *location_clone(t_location *l);
void location_set(t_location *l, uint32_t offset);
void location_copy(t_location *l1, t_location *l2);
uint32_t location_getoffset(t_location *location);
void location_readsector(t_location *l);
void location_destroy(t_location *l);

#endif

#ifndef PLANNING_H_
#define PLANNING_H_

#include "common/utils/log.h"
#include "common/collections/list.h"
#include "location.h"

typedef struct t_planning {
	t_location current;
	t_location requested;
	t_list *path;
	uint32_t time;
	t_location *next;
} t_planning;

t_planning *planning_create();
void planning_destroy(t_planning *p);
void planning_log (t_planning *p, t_log *log);

#endif

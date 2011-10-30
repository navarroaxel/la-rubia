#ifndef HEADTRACE_H_
#define HEADTRACE_H_

#include <stdio.h>
#include "common/utils/log.h"
#include "common/collections/list.h"
#include "location.h"

typedef struct t_headtrace_cylinder {
	uint16_t start;
	uint16_t end;
	int8_t step;
} t_headtrace_cylinder;

typedef struct t_headtrace {
	t_location current;
	t_location requested;
	t_headtrace_cylinder *cylinder_path1;
	t_headtrace_cylinder *cylinder_path2;
	uint16_t limitsector;
	uint32_t time;
	t_location *next;
} t_headtrace;

t_headtrace *headtrace_create();
void headtrace_destroy(t_headtrace *trace);
void waiting_log(t_blist *waiting, t_log *log);
void inprogress_log(t_list *inprogress, t_log *log);
void headtrace_log (t_headtrace *trace, t_log *log);
void headtrace_setcylinderpath(t_headtrace *trace, uint16_t start, uint16_t end, int8_t step);
t_headtrace_cylinder *headtrace_cylinder_create();
char *cylinders_string(t_headtrace_cylinder *path, uint16_t sector);
char *sectors_string(uint16_t start, uint16_t end, uint16_t limit,uint16_t cylinder);

#endif

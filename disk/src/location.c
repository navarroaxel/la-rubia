#include "location.h"

t_location *location_create(uint32_t offset) {
	t_location *l = malloc(sizeof(t_location));
	location_set(l, offset);
	return l;
}

void location_set(t_location *l, uint32_t offset) {
	l->cylinder = getcylinder(offset);
	l->sector = getsector(offset);
}

void location_copy(t_location *l1, t_location *l2) {
	l2->cylinder = l1->cylinder;
	l2->sector = l1->sector;
}

t_location *location_clone(t_location *l) {
	t_location *x = malloc(sizeof(t_location));
	x->cylinder = l->cylinder;
	x->sector = l->sector;
	return x;
}

uint32_t location_getoffset(t_location *location) {
	return getoffset(location->cylinder, location->sector);
}

void location_readsector(t_location *l){
	if(islimitsector(l->sector))
		l->sector = 0;
	else
		l->sector += 1;
}

void location_destroy(t_location *l) {
	free(l);
}

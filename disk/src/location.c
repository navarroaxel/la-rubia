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

int locations_string(t_list *locations, char *s) {
	int i = 0;
	void closure(void *data) {
		if (i != 0)
			s[i++] = ',';

		t_location *l = data;
		i += location_string(l, s + i);
	}

	collection_list_iterator(locations, closure);
	return i;
}

int location_string(t_location *l, char *s) {
	if (l == NULL) {
		*s = '\0';
		return 0;
	}

	return sprintf(s, "%i:%i", l->cylinder, l->sector);
}

void location_destroy(t_location *l) {
	free(l);
}

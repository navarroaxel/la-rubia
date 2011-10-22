#include "head.h"

struct t_disk_config disk_data;
t_location *current;
int movement = 1;
extern config_disk * config;

void init_head(t_blist *waiting, t_blist *processed, t_log *logFile) {
	current = location_create(0);
	init_disk();

	pthread_attr_t attr;
	pthread_t head_id;

	struct queues *q = (struct queues *) malloc(sizeof(struct queues));
	q->waiting = waiting;
	q->processed = processed;
	q->logFile = logFile;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&head_id, &attr, &head_cscan, q);
	pthread_attr_destroy(&attr);
}

void *head_cscan(void *args) {
	struct queues *q = (struct queues *) args;
	uint16_t refcylinder;
	t_disk_operation *e;
	t_planning *p;

	int getnextoperation(void *data) {
		return getcylinder(((t_disk_operation *) data)->offset) >= refcylinder;
	}

	while (true) {
		refcylinder = current->cylinder;
		e = (t_disk_operation *) collection_blist_popfirst(q->waiting,
				getnextoperation);
		if (e == NULL) {
			refcylinder = 0;
			e = (t_disk_operation *) collection_blist_popfirst(q->waiting,
					getnextoperation);
		}

		p = head_cscanmove(e->offset);
		//planning_log(p, q->logFile);
		planning_destroy(p);

		e->result = e->read ? disk_read(current, &e->data) : disk_write(current, &e->data);

		collection_blist_push(q->processed, e);
	}
	return NULL;
}

t_location *head_currentlocation(void) {
	return location_clone(current);
}

t_planning *head_cscanmove(uint32_t requested) {
	t_planning *p = planning_create();
	location_copy(current, &p->current);
	location_set(&p->requested, requested);
	while (current->cylinder != p->requested.cylinder) {
		if (islimitcylinder(current->cylinder))
			current->cylinder = 0;
		else {
			current->cylinder += movement;
			p->time += 2; //track time
		}
		collection_list_add(p->path, location_clone(current));
	}

	while (current->sector != p->requested.sector) {
		if (islimitsector(current->sector))
			current->sector = 0;
		else
			current->sector += 1;
		p->time += 1; //sector time
		collection_list_add(p->path, location_clone(current));
	}

	return p;
}

// ***
// *** TERCER CHECKPOINT!! ***
// ***
void *head_fscan(void *args) {
	struct queues *q = (struct queues *) args;
	t_list *inprogress = collection_list_create();
	t_disk_operation *e;

	int getnearestsectorasc(void *data) {
		//TODO: Cambiar 0 por la posicion del cabezal.
		return ((t_disk_operation *) data)->offset >= 0;
	}
	int getnearestsectordesc(void *data) {
		//TODO: Cambiar 0 por la posicion del cabezal.
		return ((t_disk_operation *) data)->offset <= 0;
	}

	while (true) {
		collection_blist_move(q->waiting, inprogress);

		while (collection_list_size(inprogress) > 0) {
			e = (t_disk_operation *) collection_list_popfirst(inprogress,
					getnearestsectorasc);

			if (e->read)
				e->result = disk_read(current, &e->data);
			else
				e->result = disk_write(current, &e->data);

			collection_blist_push(q->processed, e);
		}
	}
	return NULL;
}

void init_disk() {
	strcpy(disk_data.path, config->volumeFilePath);
	int file;
	uint32_t filesize;
	if ((file = open(disk_data.path, O_RDWR)) > 0) {
		filesize = lseek(file, 0, SEEK_END);
	} else {
		fprintf(stderr, "Error opening input file"), exit(1);
	}
	if ((disk_data.diskFile = mmap(0, filesize, PROT_READ | PROT_WRITE
	, MAP_SHARED, file, 0)) == (void *) -1)
		fprintf(stderr, "Error mapping input file"), exit(1);
}

int disk_read(t_location *location, t_sector *sector) {
	//TODO: delay tracktime & readtime
	memcpy(sector, disk_data.diskFile + location_getoffset(location) * DISK_SECTOR_SIZE,
	sizeof(t_sector));
	location_readsector(location);
	return DISK_RESULT_SUCCESS;
}

int disk_write(t_location *location, t_sector *sector) {
	//TODO: delay tracktime & writetime
	memcpy(disk_data.diskFile + location_getoffset(location) * DISK_SECTOR_SIZE, sector, sizeof(t_sector));
	location_readsector(location);
	return DISK_RESULT_SUCCESS;
}

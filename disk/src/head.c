#include "head.h"

struct t_disk_config disk_data;
t_location *current;
int movement = 1;
extern config_disk * config;

void init_head(t_blist *waiting, t_blist *processed, t_log *log) {
	current = location_create(0);
	init_disk();

	pthread_attr_t attr;
	pthread_t head_id;

	struct queues *q = (struct queues *) malloc(sizeof(struct queues));
	q->waiting = waiting;
	q->processed = processed;
	q->log = log;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	if (strcmp(config->algorithm, "CSCAN") == 0)
		pthread_create(&head_id, &attr, &head_cscan, q);
	else if (strcmp(config->algorithm, "FSCAN") == 0)
		pthread_create(&head_id, &attr, &head_fscan, q);
	else
		perror("Algoritmo de planificacion incorrecto");

	pthread_attr_destroy(&attr);
}

void *head_cscan(void *args) {
	struct queues *q = (struct queues *) args;
	uint16_t refcylinder;
	t_disk_operation *e, *nextop;

	int getnextoperation(void *data) {
		return getcylinder(((t_disk_operation *) data)->offset) >= refcylinder;
	}

	while (true) {
		refcylinder = current->cylinder;
		waiting_log(q->waiting, q->log);
		e = collection_blist_popfirst(q->waiting, getnextoperation);
		if (e == NULL) {
			refcylinder = 0;
			e = (t_disk_operation *) collection_blist_popfirst(q->waiting,
					getnextoperation);
		}

		e->headtrace = head_cscanmove(e->offset);

		e->result =
				e->read ?
						disk_read(current, &e->data) :
						disk_write(current, &e->data);

		refcylinder = current->cylinder;
		nextop = collection_blist_getfirst(q->waiting, getnextoperation);
		if (nextop == NULL) {
			refcylinder = 0;
			nextop = collection_blist_getfirst(q->waiting, getnextoperation);
		}

		if (nextop != NULL) {
			location_set(&e->headtrace->next, nextop->offset);
			e->headtrace->hasnext = true;
		}

		collection_blist_push(q->processed, e);
	}
	return NULL;
}

t_location *head_currentlocation(void) {
	return location_clone(current);
}

t_headtrace *head_cscanmove(uint32_t requested) {
	t_headtrace *trace = headtrace_create();
	location_copy(current, &trace->current);
	location_set(&trace->requested, requested);
	if (trace->requested.cylinder > trace->current.cylinder) {
		headtrace_setcylinderpath(trace, trace->current.cylinder,
				trace->requested.cylinder, 1);
		trace->time = (trace->requested.cylinder - trace->current.cylinder)
				* config->jumpTime;
	} else if (trace->requested.cylinder < trace->current.cylinder) {
		headtrace_setcylinderpath(trace, trace->current.cylinder,
				getlimitcylinder(), 1);
		headtrace_setcylinderpath(trace, 0, trace->requested.cylinder, 1);
		trace->time = (getlimitcylinder() - trace->current.cylinder
				+ trace->requested.cylinder) * config->jumpTime;
	}

	trace->sectors = config->sectors;
	if (trace->requested.sector >= trace->current.sector)
		trace->time += (trace->requested.sector - trace->current.sector)
				* getsectortime();
	else
		trace->time += (getlimitsector() - trace->current.sector
				+ trace->current.sector) * getsectortime();

	location_set(current, requested);
	return trace;
}

// ***
// *** TERCER CHECKPOINT!! ***
// ***
void *head_fscan(void *args) {
	uint16_t refcylinder;
	t_disk_operation *e, *nextop;
	bool asc = true;
	struct queues *q = args;
	t_list *inprogress = collection_list_create();

	int getnearestsectorasc(void *data) {
		return getcylinder(((t_disk_operation *) data)->offset) >= refcylinder;
	}
	int getnearestsectordesc(void *data) {
		return getcylinder(((t_disk_operation *) data)->offset) <= refcylinder;
	}

	while (true) {
		collection_blist_move(q->waiting, inprogress);

		while (collection_list_size(inprogress) > 0) {
			refcylinder = current->cylinder;
			inprogress_log(inprogress, q->log);
			e = collection_list_popfirst(inprogress,
					asc ? getnearestsectorasc : getnearestsectordesc);

			if (e == NULL) {
				refcylinder = asc ? config->cylinders : 0;

				e = collection_list_popfirst(inprogress,
						!asc ? getnearestsectorasc : getnearestsectordesc);
				e->headtrace = head_fscanmove(e->offset, asc);
				asc = !asc;
			} else
				e->headtrace = head_fscanmove(e->offset, asc);

			e->result =
					e->read ?
							disk_read(current, &e->data) :
							disk_write(current, &e->data);

			refcylinder = current->cylinder;
			nextop = collection_list_getfirst(inprogress,
					asc ? getnearestsectorasc : getnearestsectordesc);
			if (nextop == NULL) {
				refcylinder = asc ? config->cylinders : 0;
				nextop = collection_list_getfirst(inprogress,
						!asc ? getnearestsectorasc : getnearestsectordesc);
			}

			if (nextop != NULL) {
				location_set(&e->headtrace->next, nextop->offset);
				e->headtrace->hasnext = true;
			}

			collection_blist_push(q->processed, e);
		}
	}
	return NULL;
}

t_headtrace *head_fscanmove(uint32_t requested, bool asc) {
	t_headtrace *trace = headtrace_create();
	location_copy(current, &trace->current);
	location_set(&trace->requested, requested);
	if (asc) {
		if (trace->requested.cylinder > trace->current.cylinder) {
			headtrace_setcylinderpath(trace, trace->current.cylinder,
					trace->requested.cylinder, 1);
			trace->time = (trace->requested.cylinder - trace->current.cylinder)
					* config->jumpTime;
		} else if (trace->requested.cylinder < trace->current.cylinder) {
			headtrace_setcylinderpath(trace, trace->current.cylinder,
					getlimitcylinder(), 1);
			headtrace_setcylinderpath(trace, getlimitcylinder(),
					trace->requested.cylinder, -1);
			trace->time = (2 * getlimitcylinder() - trace->current.cylinder
					- trace->requested.cylinder) * config->jumpTime;
		}
	} else {
		if (trace->requested.cylinder < trace->current.cylinder) {
			headtrace_setcylinderpath(trace, trace->current.cylinder,
					trace->requested.cylinder, -1);
			trace->time = (trace->current.cylinder - trace->requested.cylinder)
					* config->jumpTime;
		} else if (trace->requested.cylinder > trace->current.cylinder) {
			headtrace_setcylinderpath(trace, trace->current.cylinder, 0, -1);
			headtrace_setcylinderpath(trace, 0, trace->requested.cylinder, 1);
			trace->time = (2 * getlimitcylinder() - trace->current.cylinder
					- trace->requested.cylinder) * config->jumpTime;
		}
	}

	trace->sectors = config->sectors;
	if (trace->requested.sector >= trace->current.sector)
		trace->time += (trace->requested.sector - trace->current.sector)
				* getsectortime();
	else
		trace->time += (getlimitsector() - trace->current.sector
				+ trace->current.sector) * getsectortime();

	location_set(current, requested);
	return trace;
}

void init_disk() {
	strcpy(disk_data.path, config->volumeFilePath);
	int file;
	if ((file = open(disk_data.path, O_RDWR)) > 0) {
		disk_data.fileSize = lseek(file, 0, SEEK_END);
	} else {
		fprintf(stderr, "Error opening input file"), exit(1);
	}
	if ((disk_data.diskFile = mmap(0, disk_data.fileSize, PROT_READ | PROT_WRITE
	, MAP_SHARED, file, 0)) == (void *) -1)
		fprintf(stderr, "Error mapping input file"), exit(1);
	madvise(disk_data.diskFile, disk_data.fileSize, MADV_SEQUENTIAL);
}

int disk_read(t_location *location, t_sector *sector) {
	sleep(config->readTime);
	memcpy(sector,
			disk_data.diskFile + location_getoffset(location) * DISK_SECTOR_SIZE
			, sizeof(t_sector));
	location_readsector(location);
	return DISK_RESULT_SUCCESS;
}

int disk_write(t_location *location, t_sector *sector) {
	static int sectorsWritten;
	sectorsWritten++;
	sleep(config->writeTime);
	memcpy(disk_data.diskFile + location_getoffset(location) * DISK_SECTOR_SIZE
	, sector, sizeof(t_sector));
	if (sectorsWritten == 8192){
		msync(disk_data.diskFile , disk_data.fileSize, MS_SYNC);
		sectorsWritten=0;
	}

	location_readsector(location);
	return DISK_RESULT_SUCCESS;
}

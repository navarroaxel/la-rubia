#include "head.h"

struct t_disk_config disk_data;
int movement = 1;

void init_head(t_blist *waiting, t_blist *processed) {
	init_disk();

	pthread_attr_t attr;
	pthread_t head_id;

	struct queues *q = (struct queues *) malloc(sizeof(struct queues));
	q->waiting = waiting;
	q->processed = processed;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&head_id, &attr, &head_cscan, q);
	pthread_attr_destroy(&attr);
}

void *head_cscan(void *args) {
	struct queues *q = (struct queues *) args;
	t_disk_operation *e;

	int getnextoperation(void *data) {
		return getcylinder(((t_disk_operation *)data)->offset) >= current->cylinder;
	}

	while (true) {
		findnextoperation: e = (t_disk_operation *)collection_blist_popfirst(q->waiting, getnextoperation);
		if (e == NULL){
			current->cylinder = 0;
			goto findnextoperation;
		}

		if (e->read)
			e->result = disk_read(e->offset, &e->data);
		else
			e->result = disk_write(e->offset, &e->data);

		collection_blist_push(q->processed, e);
	}
	return NULL;
}

t_list *head_cscanmove(struct location *current, struct location *next) {
	t_list *path = collection_list_create();

	while (current->cylinder != next->cylinder) {
		if (islimitcylinder(current->cylinder))
			current->cylinder = 0;
		else
			current->cylinder += movement;
		collection_list_add(path, location_clone(current));
	}

	while (current->sector != next->sector) {
		if (islimitsector(current->sector))
			current->sector = 0;
		else
			current->sector += 1;
		collection_list_add(path, location_clone(current));
	}

	return path;
}

location *location_clone(location *l) {
	location *x = malloc(sizeof(location));
	x->cylinder = l->cylinder;
	x->sector = l->sector;
	return x;
}

int islimitcylinder(int cylinder) {
	//TODO: cambiar por archivo de config.
	return cylinder == 10;
}

int islimitsector(int sector) {
	//TODO: cambiar por archivo de config.
	return sector == 10;
}

int getcylinder(uint32_t offset) {
	//TODO: Tomar consts de config file.
	return offset / (1 * 10); //head * sector
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
				e->result = disk_read(e->offset, &e->data);
			else
				e->result = disk_write(e->offset, &e->data);

			collection_blist_push(q->processed, e);
		}
	}
	return NULL;
}

void init_disk() {
	strcpy(disk_data.path, "/home/utn_so/fat32.disk");
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

int disk_read(uint32_t offset, t_sector *sector) {
	//TODO: head object with position
	//TODO: delay tracktime & readtime
	memcpy(sector, disk_data.diskFile + offset * DISK_SECTOR_SIZE,
	sizeof(t_sector));
	return DISK_RESULT_SUCCESS;
}

int disk_write(uint32_t offset, t_sector *sector) {
	//TODO: delay tracktime & writetime
	memcpy(disk_data.diskFile + offset, sector, sizeof(t_sector));
	return DISK_RESULT_SUCCESS;
}

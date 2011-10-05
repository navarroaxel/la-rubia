#include "head.h"

struct t_disk_config disk_data;

void init_head(t_blist *operations, t_blist *ready) {
	init_disk();

	pthread_attr_t attr;
	pthread_t head_id;

	struct queues *q = (struct queues *) malloc(sizeof(struct queues));
	q->operations = operations;
	q->readyQueue = ready;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&head_id, &attr, &head_fscan, q);
	pthread_attr_destroy(&attr);
}

void *head_cscan(void *args) {
	struct queues *q = (struct queues *) args;
	t_disk_operation *e;
	while (true) {
		e = (t_disk_operation *) collection_blist_pop(q->operations);
		if (e->read)
			e->result = disk_read(e->offset, &e->data);
		else
			e->result = disk_write(e->offset, &e->data);

		collection_blist_push(q->readyQueue, e);
	}
	return NULL;
}

void *head_fscan(void *args){
	struct queues *q = (struct queues *) args;
	t_list *waiting_list = collection_list_create();
	t_disk_operation *e;
	while (true) {
		collection_blist_move(q->operations, waiting_list);

		while (collection_list_size(waiting_list) > 0) {
			e = (t_disk_operation *) collection_list_get(waiting_list, 0);
			collection_list_remove(waiting_list, 0);

			if (e->read)
				e->result = disk_read(e->offset, &e->data);
			else
				e->result = disk_write(e->offset, &e->data);

			collection_blist_push(q->readyQueue, e);
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
	if ((disk_data.diskFile = mmap(0, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, file, 0)) == (void *) -1)
		fprintf(stderr, "Error mapping input file"), exit(1);
}

int disk_read(uint32_t offset, t_sector *sector) {
	//TODO: head object with position
	//TODO: delay tracktime & readtime
	memcpy(sector, disk_data.diskFile + offset, sizeof(t_sector));
	return DISK_RESULT_SUCCESS;
}

int disk_write(uint32_t offset, t_sector *sector) {
	//TODO: delay tracktime & writetime
	memcpy(disk_data.diskFile + offset, sector, sizeof(t_sector));
	return DISK_RESULT_SUCCESS;
}

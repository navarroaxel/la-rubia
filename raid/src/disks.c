#include "disks.h"
#include "disk.h"

int disk_id = 1;
t_list *disks;
uint32_t raidoffsetlimit;
void disks_init(void) {
	disks = collection_list_create();
}

t_disk *disks_register(char *name, t_socket_client *client, t_list *waiting, t_log *log) {
	t_disk *dsk = malloc(sizeof(t_disk));

	sem_init(&dsk->pendingsMutex, 0, 1);
	memcpy(&dsk->name, name, strlen(name) + 1);
	dsk->id = disk_id;
	dsk->operations = waiting;
	dsk->log = log;
	dsk->client = client;
	dsk->pendings = 0;
	disk_id <<= 1;

	pthread_t thread;
	pthread_create(&thread, NULL, &disk, dsk);

	dsk->thread = thread;
	collection_list_add(disks, dsk);
	return dsk;
}

t_disk *disks_getidledisk(uint32_t offset) {
	int count = INT_MAX;
	struct t_disk *dsk;
	void find_idledisk(void *data) {
		t_disk *d = data;
		if (offset < d->offsetlimit && d->pendings < count) {
			dsk = d;
			count = d->pendings;
		}
	}

	collection_list_iterator(disks, find_idledisk);
	return dsk;
}

void disks_verifystate(){
	bool valid = false;
	void validator(void *data) {
		t_disk *dsk = data;
		if (dsk->offsetlimit == raidoffsetlimit)
			valid = true;
	}

	collection_list_iterator(disks, validator);

	if (!valid){
		perror("No hay discos en estado valido");
		exit(EXIT_FAILURE);
	}
}

t_disk *disks_getbyname(char *name){
	int namecomparer(void *data){
		return strcmp(name, ((t_disk *)data)->name) == 0;
	}

	return collection_list_getfirst(disks, namecomparer);
}

int disks_size(void) {
	return collection_list_size(disks);
}

void disks_foreach(void(*closure)(void*)) {
	collection_list_iterator(disks, closure);
}

void disks_remove(t_disk *disk) {
	int removedisk(void *data) {
		return ((t_disk *) data)->id == disk->id;
	}

	collection_list_removeByClosure2(disks, removedisk);
}

void disks_destroy(t_disk *disk) {
	sem_destroy(&disk->pendingsMutex);
	sockets_destroyClient(disk->client);
	free(disk);
}

void disk_decreasepending(t_disk *dsk){
	sem_wait(&dsk->pendingsMutex);
	dsk->pendings--;
	sem_post(&dsk->pendingsMutex);
}

void disk_increasepending(t_disk *dsk){
	sem_wait(&dsk->pendingsMutex);
	dsk->pendings++;
	sem_post(&dsk->pendingsMutex);
}

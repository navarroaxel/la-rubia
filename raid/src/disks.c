#include "disks.h"
#include "disk.h"

int disk_id = 1;
t_list *disks;

void disks_init(void) {
	disks = collection_list_create();
}

t_disk *disks_register(char *name, t_socket_client *client, t_list *waiting, t_log *log) {
	struct t_disk *dsk = malloc(sizeof(struct t_disk));

	//TODO: validate duplicated name.

	memcpy(&dsk->name, name, strlen(name) + 1);
	dsk->id = disk_id;
	dsk->operations = waiting;
	dsk->log = log;
	dsk->client = client;

	disk_id <<= 1;

	pthread_t thread;
	pthread_create(&thread, NULL, &disk, dsk);

	dsk->thread = thread;
	collection_list_add(disks, dsk);
	return dsk;
}

t_disk *disks_getidledisk() {
	int count = INT_MAX;
	struct t_disk *disk;
	void find_idledisk(void *data) {
		t_disk *d = data;
		if (d->pendings < count) {
			disk = data;
			count = d->pendings;
		}
	}

	collection_list_iterator(disks, find_idledisk);
	return disk;
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
	sockets_destroyClient(disk->client);
	free(disk);
}

#include "disks.h"
#include "disk.h"

int disk_id = 1;
t_list *disks;

void registerdisk(char *name, t_socket_client *client) {
	struct t_disk *dsk = malloc(sizeof(struct t_disk));

	//TODO: validate duplicated name.

	memcpy(&dsk->name, name, strlen(name) + 1);
	dsk->id = disk_id;
	dsk->operations = NULL;
	dsk->client = client;

	disk_id <<= 1;

	pthread_t thread;
	pthread_create(&thread, NULL, &disk, dsk);

	dsk->thread = thread;
	collection_list_add(disks, dsk);
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
	sockets_destroy(disk->client);
	free(disk);
}

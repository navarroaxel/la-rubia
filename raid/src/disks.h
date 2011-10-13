#ifndef DISKS_H_
#define DISKS_H_

#include <limits.h>
#include <pthread.h>
#include "common/collections/list.h"
#include "common/utils/sockets.h"
#include "operations.h"

typedef struct t_disk{
	uint32_t id;
	char name[13];
	pthread_t thread;
	t_list *operations;
	t_socket_client *client;
	int pendings; //TODO: Revisar de poner un semaphore para hacer esta operacion segura.
} t_disk;

t_disk *disks_register(char *name, t_socket_client *client);
t_disk *disks_getidledisk();
int disks_size(void);
void disks_foreach(void(*closure)(void*));
void disks_remove(t_disk *disk);
void disks_destroy(t_disk *disk);

#endif

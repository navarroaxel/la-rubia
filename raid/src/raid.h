#ifndef RAID_H_
#define RAID_H_

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <limits.h>
#include "disk.h"
#include "common/nipc.h"
#include "common/utils/sockets.h"
#include "common/collections/blist.h"

struct t_disk {
	char id[255];
	pthread_t thread;
	t_blist *waiting;
	t_socket_client *client;
};

t_list *disks;
void registerdisk(char *id, t_socket_client *client);
void enqueueoperation(t_nipc *nipc, t_socket_client *client);

#endif

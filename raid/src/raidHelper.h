#ifndef RAIDHELPER_H_
#define RAIDHELPER_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "common/nipc.h"

typedef struct t_disk{
	uint32_t id;
	char name[13];
	pthread_t thread;
	t_list *waiting;
	t_socket_client *client;
	int pendingreads;
} t_disk;

typedef struct t_operation {
	bool read;
	uint32_t offset;
	uint8_t data[DISK_SECTOR_SIZE];
	uint32_t disk;
	t_socket_client *client;
} t_operation;

t_operation *operation_create(t_nipc *nipc);
void operation_destroy(t_operation *operation);

#endif

#ifndef DISK_H_
#define DISK_H_

#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include "common/utils/sockets.h"
#include "common/collections/blist.h"
#include "common/nipc.h"
#include "common/utils/commands.h"

typedef uint8_t t_sector[DISK_SECTOR_SIZE];

typedef struct t_disk_operation {
	bool read;
	uint32_t offset;
	t_sector data;
	uint8_t result;
	t_socket_client *client;
} t_disk_operation;

struct t_disk_config {
	char path[255];
	void *diskFile;
};

void info(void *context, t_array *args);
void clean(void *context, t_array *args);
void trace(void *context, t_array *args);

#endif

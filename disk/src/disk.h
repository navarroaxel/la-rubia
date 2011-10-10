#ifndef DISK_H_
#define DISK_H_

#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include "common/utils/sockets.h"
#include "common/collections/blist.h"
#include "common/nipc.h"

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

void console(void);
void info(char *input);
void clean(char *input);
void trace(char *input);

#endif

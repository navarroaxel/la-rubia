#ifndef DISK_H_
#define DISK_H_

#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include "common/utils/sockets.h"
#include "common/collections/blist.h"
#include "common/nipc.h"

typedef struct t_disk_operation {
	bool read;
	uint32_t offset;
	uint8_t data[DISK_SECTOR_SIZE];
	t_socket_client *client;
} t_disk_operation;

struct t_disk_config{
	char path[255];
	void * diskFile;
};
typedef uint8_t t_sector[512];
void enqueueOperation(t_blist *list, t_disk_operation *op);
void socketlisten(t_blist *list);
void init_head(t_blist *operations, t_blist *ready);
void init_dispatcher(t_blist *ready);
void *dispatcher(void *args);
void *head(void *args);
t_disk_operation *getdiskoperation(t_nipc *nipc, t_socket_client *client);
void init_disk();
void disk_read(uint32_t sectorN,t_sector * sector );
void disk_write(uint32_t sectorN,t_sector * sector );

#endif

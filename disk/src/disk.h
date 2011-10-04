#ifndef DISK_H_
#define DISK_H_

#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include "common/utils/sockets.h"
#include "common/collections/blist.h"

typedef struct t_disk_operation {
	bool read;
	uint32_t offset;
	uint8_t data[512];
	t_socket_client *client;
} t_disk_operation;

void enqueueOperation(t_blist *list, t_disk_operation *op);
void socketlisten(t_blist *list);
void init_head(t_blist *operations, t_blist *ready);
void init_dispatcher(t_blist *ready);
void *dispatcher(void *args);
void *head(void *args);

#endif

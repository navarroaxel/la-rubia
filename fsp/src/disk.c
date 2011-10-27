/*
 * disk.c
 *
 *  Created on: Sep 2, 2011
 *      Author: nico
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include "fatTypes.h"
#include "disk.h"
#include "common/nipc.h"
#include "common/utils/sockets.h"
#include "common/utils/config.h"
#include "assert.h"

/**
 * Read Sector into buffer;
 */
t_socket_client *client;
sem_t semaforo;
static int disk_is_initialized = 0;
extern config_fsp * config;

int disk_initialize() {
	sem_init(&semaforo, 0, 1);
	client = sockets_createClient(config->bindIp, config->bindPort);
	sockets_connect(client, config->diskIp,config->diskPort);
	t_nipc * nipc = nipc_create(NIPC_HANDSHAKE);
	nipc_send(nipc,client);
	nipc_destroy(nipc);
	t_socket_buffer *buffer = sockets_recv(client);
	if(buffer==NULL){
		printf("Error de Handshake");
		exit(0);
	}
	nipc = nipc_deserializer(buffer);
	if (nipc->length!=0){
		printf("Error de Handshake");
		exit(0);
	}
	nipc_destroy(nipc);
	sockets_bufferDestroy(buffer);
	disk_is_initialized = 1;
	return 0;
}

int disk_isInitialized() {
	return disk_is_initialized;
}

int disk_readSector(uint32_t sector, t_sector buf) {
	sem_wait(&semaforo);
	t_disk_readSectorRq *rq = malloc(sizeof(t_disk_readSectorRq));
	rq->offset = sector;

	t_nipc *nipc = nipc_create(NIPC_READSECTOR_RQ);
	t_disk_readSectorRs *rs;
	nipc_setdata(nipc, rq, sizeof(t_disk_readSectorRq));

	t_socket_buffer *buffer = (t_socket_buffer *) nipc_serializer(nipc);
	sockets_send(client, buffer->data, buffer->size);
	sockets_bufferDestroy(buffer);
	nipc_destroy(nipc);

	buffer = sockets_recv(client);
	if(buffer==NULL){
		printf("%d",sector);
		exit(0);
	}
	nipc = nipc_deserializer(buffer);
	sockets_bufferDestroy(buffer);
	rs = (t_disk_readSectorRs *)nipc->payload;
	memcpy(buf, rs->data,FAT_SECTOR_SIZE);
	nipc_destroy(nipc);
	sem_post(&semaforo);
	return 1;
}

int disk_writeSector(uint32_t sector, t_sector buf) {
	sem_wait(&semaforo);
	t_disk_writeSectorRq *rq = malloc(sizeof(t_disk_writeSectorRq));
	rq->offset = sector;
	memcpy(rq->data,buf,FAT_SECTOR_SIZE);

	t_nipc *nipc = nipc_create(NIPC_WRITESECTOR_RQ);
	t_disk_writeSectorRs *rs;
	nipc_setdata(nipc, rq, sizeof(t_disk_writeSectorRq));

	t_socket_buffer *buffer = (t_socket_buffer *) nipc_serializer(nipc);
	sockets_send(client, buffer->data, buffer->size);
	sockets_bufferDestroy(buffer);
	nipc_destroy(nipc);

	buffer = sockets_recv(client);
	if(buffer==NULL){
		printf("%d",sector);
		exit(0);
	}
	nipc = nipc_deserializer(buffer);
	sockets_bufferDestroy(buffer);
	rs = (t_disk_writeSectorRs *)nipc->payload;
	nipc_destroy(nipc);
	sem_post(&semaforo);
	return 1;
}

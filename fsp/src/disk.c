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
#include "common/utils/sockets.h"
#include "disk.h"
#include "common/nipc.h"
#include "common/utils/config.h"
#include "common/utils/array.h"
#include "assert.h"

int connPoolSize;
sem_t poolResources,ConnGetMutex;
static int disk_is_initialized = 0;
extern config_fsp * config;
t_disk_connection * connPool;

int disk_initialize() {
	connPoolSize = config->maxConnections;
	sem_init(&poolResources, 0, connPoolSize);
	sem_init(&ConnGetMutex, 0, 1);
	connPool = (t_disk_connection *) malloc(sizeof(t_disk_connection)*connPoolSize);
	memset (connPool,0,sizeof(t_disk_connection)*connPoolSize);
	disk_is_initialized = 1;
	return 0;
}

int disk_isInitialized() {
	return disk_is_initialized;
}
t_disk_connection * disk_getConnection(){
	sem_wait(&poolResources);
	sem_wait(&ConnGetMutex);
	int i;
	t_disk_connection * conn = NULL;
	for (i=0;i<connPoolSize;i++){
		if(connPool[i].inUse) continue;

		if(!connPool[i].connected){
			disk_connect(&connPool[i],i);
		}

		connPool[i].inUse =1;
		conn = &connPool[i];
		break;
	}
	assert(conn!=NULL);
	sem_post(&ConnGetMutex);
	return conn;
}

void disk_ReleaseConnection(t_disk_connection * conn){
	conn->inUse=0;
	sem_post(&poolResources);
}

int disk_connect(t_disk_connection * conn,uint16_t portOffset){
	conn->client = sockets_createClient(config->bindIp, config->bindPort +portOffset);
	sockets_connect(conn->client, config->diskIp,config->diskPort);
	t_nipc * nipc = nipc_create(NIPC_HANDSHAKE);
	nipc_send(nipc,conn->client);
	nipc_destroy(nipc);
	t_socket_buffer *buffer = sockets_recv(conn->client);
	if(buffer==NULL){
		conn->connected=0;
		return 0;
	}
	nipc = nipc_deserializer(buffer,0);
	if (nipc->length!=0){
		conn->connected=0;
		return 0;
	}
	nipc_destroy(nipc);
	sockets_bufferDestroy(buffer);
	conn->connected =1;
	return 1;
}
void disk_freeConn(t_disk_connection * conn){
	if (conn->client!=NULL)
		sockets_destroyClient(conn->client);
}

void disk_cleanup(void){
	int i;
	for(i=0;i<connPoolSize;i++){
		disk_freeConn(connPool+i);
	}
	free(connPool);
}

int disk_readSectors(uint32_t sectorsStart,uint32_t length, uint8_t * buf) {
	t_disk_connection * conn = disk_getConnection();
	uint32_t i;
	t_nipc *nipc;
	t_socket_buffer *buffer;
	t_disk_readSectorRs *rs;
	uint32_t offsetInBuffer=0;
	for(i=0;i<length;i++){
		t_disk_readSectorRq *rq = malloc(sizeof(t_disk_readSectorRq));
		rq->offset = sectorsStart + i;
		nipc = nipc_create(NIPC_READSECTOR_RQ);
		nipc_setdata(nipc, rq, sizeof(t_disk_readSectorRq));

		buffer = (t_socket_buffer *) nipc_serializer(nipc);
		sockets_send(conn->client, buffer->data, buffer->size);
		sockets_bufferDestroy(buffer);
		nipc_destroy(nipc);
	}
	for(i=0;i<length;i++){
		buffer = sockets_recv(conn->client);
		assert(buffer!=NULL);
		nipc = nipc_deserializer(buffer,0);
		rs = (t_disk_readSectorRs *)nipc->payload;
		offsetInBuffer = (rs->offset - sectorsStart)*FAT_SECTOR_SIZE;
		memcpy(buf+offsetInBuffer, rs->data,FAT_SECTOR_SIZE);
		nipc_destroy(nipc);
		sockets_bufferDestroy(buffer);
	}
	disk_ReleaseConnection(conn);
	return 1;
}

int disk_writeSectors(uint32_t sectorsStart,uint32_t length, uint8_t * buf) {
	t_disk_connection * conn = disk_getConnection();
	uint32_t i;
	t_nipc *nipc;
	t_socket_buffer *buffer;
	t_disk_writeSectorRs *rs;
	for(i=0;i<length;i++){
		t_disk_writeSectorRq *rq = malloc(sizeof(t_disk_writeSectorRq));
		rq->offset = sectorsStart+i;
		memcpy(rq->data,buf+i*FAT_SECTOR_SIZE,FAT_SECTOR_SIZE);

		nipc = nipc_create(NIPC_WRITESECTOR_RQ);
		nipc_setdata(nipc, rq, sizeof(t_disk_writeSectorRq));

		buffer = (t_socket_buffer *) nipc_serializer(nipc);
		sockets_send(conn->client, buffer->data, buffer->size);
		sockets_bufferDestroy(buffer);
		nipc_destroy(nipc);
	}
	for(i=0;i<length;i++){
		uint32_t offsetInSocketBuffer=0;
		buffer = sockets_recv(conn->client);

		assert(buffer!=NULL);
		nipc = nipc_deserializer(buffer,offsetInSocketBuffer);
		offsetInSocketBuffer += nipc->length + sizeof(nipc->type) + sizeof(nipc->length);
		rs = (t_disk_writeSectorRs *)nipc->payload;
		nipc_destroy(nipc);

		sockets_bufferDestroy(buffer);
	}
	disk_ReleaseConnection(conn);
	return 1;
}


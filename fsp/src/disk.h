/*
 * disk.h
 *
 *  Created on: Sep 2, 2011
 *      Author: nico
 */

#ifndef DISK_H_
#define DISK_H_
#include "common/utils/sockets.h"

typedef struct {
	int connected;
	int inUse;
	t_socket_client * client;
}t_disk_connection;

//int disk_readSector(uint32_t sector, t_sector buf);
//int disk_writeSector(uint32_t sector,t_sector buf);
int disk_initialize();
int disk_isInitialized();
int disk_connect(t_disk_connection * conn,uint16_t portOffset);
t_disk_connection * disk_getConnection();
void disk_ReleaseConnection(t_disk_connection * conn);
void disk_cleanup(void);
int disk_readSectors(uint32_t sectorsStart,uint32_t length, uint8_t * buf);
int disk_writeSectors(uint32_t sectorsStart,uint32_t length, uint8_t * buf);

#endif /* DISK_H_ */

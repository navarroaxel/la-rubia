/*
 * disk.h
 *
 *  Created on: Sep 2, 2011
 *      Author: nico
 */

#ifndef DISK_H_
#define DISK_H_

int disk_readSector(uint32_t sector, sector_t *buf);
int disk_writeSector(unsigned int sector,unsigned char * buf);
int disk_initialize();
int disk_isInitialized();


#endif /* DISK_H_ */

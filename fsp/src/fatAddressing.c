/*
 * fatAddressing.c
 *
 *  Created on: Sep 5, 2011
 *      Author: nico
 */


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include "fat.h"
#include "fatAddressing.h"
#include "disk.h"
#include "cache.h"

extern t_fat_bootsector bootSector;

int fat_addressing_readCluster(uint32_t clusterNumber, t_cluster * buffer){
	t_sector sector;
	uint32_t offset;
	t_cluster cluster;
	uint32_t sectorN;

	if (!disk_isInitialized())
		disk_initialize();
	if (!cache_isInitialized())
		cache_initialize();
	if (cache_read(clusterNumber,buffer)){
		return 0;
	}
	uint32_t clusterFirstSector=clusterNumber*bootSector.sectorPerCluster;
	uint32_t sectorSize=bootSector.bytesPerSector;
	for (sectorN = 0 ; sectorN < bootSector.sectorPerCluster ; sectorN++) {
		if (!disk_readSector(clusterFirstSector + sectorN , &sector)){
			return 0;
		}else{
			offset = sectorN * sectorSize;
			//copiame la data leida en el cluster con el offset que corresponde
			memcpy(&(cluster[offset]), &sector,sectorSize); //WARNING: Aritmetica de Punteros
		}
	}
	memcpy(buffer,&cluster,FAT_CLUSTER_SIZE);
	cache_write(clusterNumber,&cluster);
	return 1;
}
int fat_addressing_writeCluster(uint32_t clusterNumber, t_cluster * cluster){
	t_sector sector;
	uint32_t offset;
	uint32_t sectorN;

	if (!disk_isInitialized())
		disk_initialize();

	uint32_t clusterFirstSector=clusterNumber*bootSector.sectorPerCluster;
	uint32_t sectorSize=bootSector.bytesPerSector;
	for (sectorN = 0 ; sectorN < bootSector.sectorPerCluster ; sectorN++) {
		offset = sectorN * sectorSize;
		memcpy(&sector,&((*cluster)[offset]),sectorSize);
		if (!disk_writeSector(clusterFirstSector + sectorN , &sector)){
			return 0;
		}
	}
	cache_write(clusterNumber,cluster);
	return 1;
}


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

int fat_addressing_readCluster(uint32_t clusterNumber, t_cluster * buffer,t_fat32_bootsector bs){
	t_sector sector;
	uint32_t offset;
	t_cluster cluster;

	if (!disk_isInitialized())
		disk_initialize();

	uint32_t clusterFirstSector=clusterNumber*bs.sectorPerCluster;
	uint32_t sectorSize=bs.bytesPerSector;
	for (uint32_t sectorN = 0 ; sectorN < bs.sectorPerCluster ; sectorN++) {
		if (!disk_readSector(clusterFirstSector + sectorN , &sector)){
			return 0;
		}else{
			offset = sectorN * sectorSize;
			//copiame la data leida en el cluster con el offset que corresponde
			memcpy(&(cluster[offset]), &sector,sectorSize); //WARNING: Aritmetica de Punteros
		}
	}
	memcpy(buffer,&cluster,4096);
	return 1;
}

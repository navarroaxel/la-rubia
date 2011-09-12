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

int fat_addressing_readCluster(uint32_t clusterNumber, cluster_t * buffer,t_fat32_bootsector bs){
	uint8_t sector[512];
	uint32_t offset;

	if (!disk_isInitialized())
		disk_initialize();

	uint32_t clusterFirstSector=clusterNumber*bs.sectorPerCluster;
	uint32_t sectorSize=bs.bytesPerSector;
	uint8_t sectorN;
	for (sectorN = 0 ; sectorN < bs.sectorPerCluster ; sectorN++) {
		if (!disk_readSector(clusterFirstSector + sectorN , &sector)){
			return 0;
		}else{
			offset = sectorN * sectorSize;
			//copiame la data leida en el cluster con el offset que corresponde
			memcpy(buffer+offset, &sector,sectorSize); //WARNING: Aritmetica de Punteros
		}
	}
	return 1;
}

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

int fat_addressing_readBlock(uint32_t blockNumber, t_block buffer){
	uint32_t i;
	uint32_t baseSector = blockNumber * FAT_BLOCK_SIZE/FAT_SECTOR_SIZE;
	if (!disk_isInitialized())
		disk_initialize();
	if(!cache_isInitialized())
		cache_initialize();
	if(cache_read(blockNumber,buffer))
		return 1;

	for(i=0;i<FAT_BLOCK_SIZE/FAT_SECTOR_SIZE;i++){
		disk_readSector(baseSector+i,&buffer[i*FAT_SECTOR_SIZE]);
	}
	cache_write(blockNumber,buffer);
	return 1;
}
int fat_addressing_writeBlock(uint32_t blockNumber, t_block buffer){
	uint32_t i;
	uint32_t baseSector = blockNumber * FAT_BLOCK_SIZE/FAT_SECTOR_SIZE;
	if (!disk_isInitialized())
		disk_initialize();
	if(!cache_isInitialized())
		cache_initialize();

	for(i=0;i<FAT_BLOCK_SIZE/FAT_SECTOR_SIZE;i++){
		disk_writeSector(baseSector+i,&buffer[i*FAT_SECTOR_SIZE]);
	}
	cache_write(blockNumber,buffer);
	return 1;
}

int fat_addressing_readCluster(uint32_t clusterNumber, t_cluster buffer){
	uint32_t blocksPerCluster = FAT_CLUSTER_SIZE / FAT_BLOCK_SIZE;
	uint32_t clusterFirstBlock = (fat_getRootDirectoryFirstCluster() + clusterNumber -2 ) * blocksPerCluster ;
	uint32_t i;
	for(i=0;i<blocksPerCluster;i++){
		fat_addressing_readBlock(clusterFirstBlock+i,&buffer[i*FAT_BLOCK_SIZE]);
	}
	return 1;

}
int fat_addressing_writeCluster(uint32_t clusterNumber, t_cluster buffer){
	uint32_t blocksPerCluster = FAT_CLUSTER_SIZE / FAT_BLOCK_SIZE;
	uint32_t clusterFirstBlock = (fat_getRootDirectoryFirstCluster() + clusterNumber -2 ) * blocksPerCluster ;
	uint32_t i;
	for (i = 0 ; i < blocksPerCluster ; i++) {
		fat_addressing_writeBlock(clusterFirstBlock+i,&buffer[i*FAT_BLOCK_SIZE]);
	}
	return 1;
}

uint32_t fat_getRootDirectoryFirstCluster(){
	return ((uint32_t) bootSector.reservedSectorCount + (uint32_t)bootSector.numberFATs * bootSector.sectorPerFAT32)/bootSector.sectorPerCluster ;
}


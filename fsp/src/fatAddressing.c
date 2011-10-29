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
int fat_addressing_readBlocks(uint32_t blockStart, uint32_t length, uint8_t * buffer){
	uint32_t sectorsPerBlock =FAT_BLOCK_SIZE/FAT_SECTOR_SIZE;
	uint32_t baseSector = blockStart * sectorsPerBlock;
	if (!disk_isInitialized())
		disk_initialize();

	disk_readSectors(baseSector,length*sectorsPerBlock,buffer);
	return 1;
}
int fat_addressing_writeBlocks(uint32_t blockStart, uint32_t length, uint8_t * buffer){
	uint32_t sectorsPerBlock =FAT_BLOCK_SIZE/FAT_SECTOR_SIZE;
	uint32_t baseSector = blockStart * FAT_BLOCK_SIZE/FAT_SECTOR_SIZE;
	if (!disk_isInitialized())
		disk_initialize();

	disk_writeSectors(baseSector,length*sectorsPerBlock,buffer);
	return 1;
}

int fat_addressing_readCluster(uint32_t clusterNumber, t_cluster buffer){
	/*if(!cache_isInitialized())
		cache_initialize();
	if(cache_read(clusterNumber,buffer))
		return 1;*/
	uint32_t blocksPerCluster = FAT_CLUSTER_SIZE / FAT_BLOCK_SIZE;
	uint32_t clusterFirstBlock = (fat_getRootDirectoryFirstCluster() + clusterNumber -2 ) * blocksPerCluster ;
	fat_addressing_readBlocks(clusterFirstBlock,blocksPerCluster,buffer);
	//cache_write(clusterNumber,buffer);
	return 1;

}
int fat_addressing_writeCluster(uint32_t clusterNumber, t_cluster buffer){
	/*if(!cache_isInitialized())
		cache_initialize();*/
	uint32_t blocksPerCluster = FAT_CLUSTER_SIZE / FAT_BLOCK_SIZE;
	uint32_t clusterFirstBlock = (fat_getRootDirectoryFirstCluster() + clusterNumber -2 ) * blocksPerCluster ;
	fat_addressing_writeBlocks(clusterFirstBlock,blocksPerCluster,buffer);
	//cache_write(clusterNumber,buffer);
	return 1;
}

uint32_t fat_getRootDirectoryFirstCluster(){
	return ((uint32_t) bootSector.reservedSectorCount + (uint32_t)bootSector.numberFATs * bootSector.sectorPerFAT32)/bootSector.sectorPerCluster ;
}


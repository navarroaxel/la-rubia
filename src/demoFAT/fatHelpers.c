#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include "fat.h"

uint32_t fat_getEntryFirstCluster(t_fat32_file_entry fileEntry ){
	// .firstClusterHigh = 0xAB
	// .firstClusterLow  = 0xCD
	// return 0xABCD
	uint32_t firstCluster;

	firstCluster = (uint32_t)fileEntry.firstClusterHigh; // Deberia hacer que 0xAB sea 0x00AB
	firstCluster = firstCluster << 8; // Pongo los bits altos en la posicion que corresponden 0xAB00
	firstCluster = firstCluster | (uint32_t) fileEntry.firstClusterLow; //Convierto el Low a uint32 y lo OR'eo bit a bit
	// 0xAB00  1010 1101 0000 0000
	// 0x00CD  0000 0000 1100 1101
	//----------------------------
	// 0xABCD  1010 1101 1100 1101

	//TODO: Ver si con la suma funciona tambien
	return firstCluster;
}

uint32_t fat_getRootDirectoryFirstCluster(t_fat32_bootsector bs){
	uint32_t reservedSectors,numberOfFATs,sectorsPerFAT;
	reservedSectors=bs.reservedSectorCount;
	numberOfFATs=bs.numberFATs;
	sectorsPerFAT=bs.sectorPerFAT32;

	return ((uint32_t) bs.reservedSectorCount + (uint32_t)bs.numberFATs * bs.sectorPerFAT32)/bs.sectorPerCluster ;
}

uint32_t fat_getFATFirstCluster(t_fat32_bootsector bs){
	return bs.reservedSectorCount/bs.sectorPerCluster;
}

uint32_t fat_getNextCluster(uint32_t currentCluster){

	return 0;
}



#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include "fatTypes.h"
#include "fatAddressing.h"
#include "fat.h"
#include <assert.h>

extern t_fat_bootsector bootSector;
uint32_t fat_getEntryFirstCluster(t_fat_file_data_entry * fileEntry ){
	// .firstClusterHigh = 0xAB
	// .firstClusterLow  = 0xCD
	// return 0xABCD
	uint32_t firstCluster;

	firstCluster = (uint32_t)(fileEntry->firstClusterHigh); // Deberia hacer que 0xAB sea 0x00AB
	firstCluster = firstCluster << 8; // Pongo los bits altos en la posicion que corresponden 0xAB00
	firstCluster = firstCluster | (uint32_t) (fileEntry->firstClusterLow); //Convierto el Low a uint32 y lo OR'eo bit a bit
	// 0xAB00  1010 1101 0000 0000
	// 0x00CD  0000 0000 1100 1101
	//----------------------------
	// 0xABCD  1010 1101 1100 1101

	//TODO: Ver si con la suma funciona tambien
	return firstCluster;
}

uint32_t fat_getRootDirectoryFirstCluster(){
	return ((uint32_t) bootSector.reservedSectorCount + (uint32_t)bootSector.numberFATs * bootSector.sectorPerFAT32)/bootSector.sectorPerCluster ;
}

uint32_t fat_getFATFirstCluster(){
	return bootSector.reservedSectorCount/bootSector.sectorPerCluster;
}

uint32_t fat_getNextCluster(uint32_t currentCluster){
	t_cluster cluster;
	uint32_t * fatCluster;
	uint32_t clusterAddress = fat_getFATFirstCluster() + currentCluster / 1024; //1024 * 4 = 4096
	fat_addressing_readCluster(clusterAddress,&cluster,bootSector);
	fatCluster = (uint32_t *) cluster;
	//assert(fatCluster[currentCluster % 1024]!=0);
	return fatCluster[currentCluster % 1024];

}

uint32_t fat_getClusterCount(t_fat_file_data_entry * file){
	uint32_t ret=1;
	uint32_t currentCluster = fat_getEntryFirstCluster(file);
	while((currentCluster = fat_getNextCluster(currentCluster)) != FAT_LASTCLUSTER)
		ret++;
	return ret;
}

t_fat_file_entry * fat_findInDir(const t_fat_file_list * dir,char * name){//TODO: Nombres Largos
	t_fat_file_list * p=dir;
	char entryName [12];
	while(p!=NULL){
		fat_getName(&p->fileEntry,entryName);
		if(strcmp(entryName,name)==0){
			return &p->fileEntry;
		}
		p=p->next;
	}
	return NULL;
}





#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include "fatTypes.h"
#include "fatAddressing.h"
#include "fat.h"
#include <assert.h>
#include "fatHelpers.h"

extern t_fat_bootsector bootSector;
extern uint32_t * fatTable;

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
	return fatTable[currentCluster];
}

int fat_fat_setValue(uint32_t clusterN,uint32_t next){
	uint32_t entriesPerCluster = FAT_CLUSTER_SIZE / FAT_FAT_ENTRY_SIZE;
	uint32_t diskCluster = fat_getFATAddressOfEntry(clusterN);
	t_cluster cluster;
	fatTable[clusterN]=next;
	memcpy(&cluster,&fatTable[clusterN - clusterN % entriesPerCluster],FAT_CLUSTER_SIZE);
	fat_addressing_writeCluster(diskCluster,&cluster);
	return 0;
}

uint32_t fat_getClusterCount(t_fat_file_data_entry * file){
	uint32_t ret=1;
	uint32_t currentCluster = fat_getEntryFirstCluster(file);
	while((currentCluster = fat_getNextCluster(currentCluster)) != FAT_LAST_CLUSTER)
		ret++;
	return ret;
}

t_fat_file_entry * fat_findInDir(const t_fat_file_list * dir,char * name){
	t_fat_file_list * p= (t_fat_file_list *)dir;
	char entryName [14];
	while(p!=NULL){
		fat_getName(&p->fileEntry,entryName);
		if(strcmp(entryName,name)==0){
			return &p->fileEntry;
		}
		p=p->next;
	}
	return NULL;
}

uint32_t fat_getNextFreeCluster(uint32_t start){
	uint32_t i;
	for (i = start; i<bootSector.totalSectors32/bootSector.sectorPerCluster;i++){
		if (fatTable[i]==FAT_FREE_CLUSTER)
			return i;
	}
	return 0;
}

uint32_t fat_getFATAddressOfEntry(uint32_t clusterN){
	uint32_t entriesPerFATCluster= FAT_CLUSTER_SIZE/FAT_FAT_ENTRY_SIZE;
	return fat_getFATFirstCluster() + clusterN / entriesPerFATCluster;
}

uint32_t fat_getFileLastCluster(t_fat_file_entry * file){
	uint32_t clusterN,temp;
	clusterN=temp=fat_getEntryFirstCluster(&file->dataEntry);
	while((temp=fat_getNextCluster(temp))!=FAT_LAST_CLUSTER){
		clusterN=temp;
	}
	return clusterN;
}

uint32_t fat_getClusterPointingTo(uint32_t clusterToFind, uint32_t clusterofChain){
	uint32_t ret=clusterofChain;
	while((clusterofChain=fat_getNextCluster(clusterofChain))!=clusterToFind){
		ret=clusterofChain;
	}
	return ret;
}
/**
 * pasar de cluster de zona datos ( lo que leo de la entrada de archivo) a cluster en el disk (offset FAT)
 */
uint32_t fat_dataClusterToDiskCluster(uint32_t dataCluster){
	return fat_getRootDirectoryFirstCluster()+ dataCluster -2;
}

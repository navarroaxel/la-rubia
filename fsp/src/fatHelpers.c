#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <ctype.h>
#include "fatTypes.h"
#include "fatAddressing.h"
#include "fat.h"
#include <assert.h>
#include "fatHelpers.h"
#include "common/utils/utils.h"
#include "semaphore.h"


extern t_fat_bootsector bootSector;
extern uint32_t * fatTable;
extern sem_t fatTableSemaphore;

uint32_t fat_getEntryFirstCluster(t_fat_file_data_entry * fileEntry ){
	// .firstClusterHigh = 0xAB
	// .firstClusterLow  = 0xCD
	// return 0xABCD
	uint32_t firstCluster;

	firstCluster = (uint32_t)(fileEntry->firstClusterHigh); // Deberia hacer que 0xAB sea 0x00AB
	firstCluster = firstCluster << 16; // Pongo los bits altos en la posicion que corresponden 0xAB00
	firstCluster = firstCluster | (uint32_t) (fileEntry->firstClusterLow); //Convierto el Low a uint32 y lo OR'eo bit a bit
	// 0xAB00  1010 1101 0000 0000
	// 0x00CD  0000 0000 1100 1101
	//----------------------------
	// 0xABCD  1010 1101 1100 1101

	return firstCluster;
}

void fat_setEntryFirstCluster(uint32_t firstCluster ,t_fat_file_data_entry * fileEntry ){
	fileEntry->firstClusterLow=firstCluster & 0x0000FFFF;
	fileEntry->firstClusterHigh=firstCluster >> 16;
	return;
}

uint32_t fat_getFATFirstBlock(){
	return bootSector.reservedSectorCount*FAT_SECTOR_SIZE/FAT_BLOCK_SIZE;
}

uint32_t fat_getNextCluster(uint32_t currentCluster){
	sem_wait(&fatTableSemaphore);
	uint32_t ret = fatTable[currentCluster];
	sem_post(&fatTableSemaphore);
	return ret;
}

int fat_fat_setValue(uint32_t clusterN,uint32_t next){

	uint32_t entriesPerBlock = FAT_BLOCK_SIZE / FAT_FAT_ENTRY_SIZE;
	uint32_t diskBlock = fat_getFATAddressOfEntry(clusterN);
	t_block block;
	sem_wait(&fatTableSemaphore);
	fatTable[clusterN]=next;
	memcpy(&block,&fatTable[clusterN - clusterN % entriesPerBlock],FAT_BLOCK_SIZE);
	fat_addressing_writeBlocks(diskBlock,1,block);
	sem_post(&fatTableSemaphore);
	return 0;
}

uint32_t fat_getClusterCount(t_fat_file_data_entry * file){
	uint32_t ret=1;
	uint32_t currentCluster = fat_getEntryFirstCluster(file);
	if(currentCluster==0) return 0;
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
		if (fat_getNextCluster(i)==FAT_FREE_CLUSTER)
			return i;
	}
	return 0;
}

uint32_t fat_getFATAddressOfEntry(uint32_t clusterN){
	uint32_t entriesPerFATBlock= FAT_BLOCK_SIZE/FAT_FAT_ENTRY_SIZE;
	return fat_getFATFirstBlock() + clusterN / entriesPerFATBlock;
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

void splitPathName(const char * path,char * dir, char * file){
	int i=0;
	char * lastSlash = strrchr(path,'/');
	while(path+i!=lastSlash){
		dir[i]=path[i];
		i++;
	}
	if (i==0){
		dir[i]='/';
		i++;
	}
	dir[i]='\0';
	strcpy(file,lastSlash+1);
}

void splitNameExtension(const char * fullName,char * name, char * extension){
	int i=0;
	char * lastDot = strrchr(fullName,'.');
	while(fullName+i!=lastDot){
		name[i]=fullName[i];
		i++;
	}
	name[i]='\0';
	strcpy(extension,lastDot+1);
}

int fat_renameFile(const char * newName, t_fat_file_entry * destinationDir, t_fat_file_entry * fileEntry){
	char shortName[9];
	char nameWithoutExtension[15];
	char extension[4];
	char fullShortName[12];
	uint16_t longName[14];
	if (fileEntry->dataEntry.attributes!=0x10){//no es un dir
		splitNameExtension(newName,nameWithoutExtension,extension);
	}else{
		strncpy(nameWithoutExtension,newName,8);
		nameWithoutExtension[14]='\0';
		strncpy(extension,"   ",3);
		extension[3]='\0';
	}
	fat_generateShortName(nameWithoutExtension,destinationDir,shortName);
	stringToUpper(extension);
	stringToUpper(shortName);
	memcpy(fileEntry->dataEntry.name,shortName,8);
	memcpy(fileEntry->dataEntry.extension,extension,3);
	if (needsLongName(newName)){
		unicode_utf8_to_utf16_inbuffer(newName,strlen(newName)+1,longName,NULL);
		memset(longName+strlen(newName)+1,0,14-(strlen(newName)+1));
		memcpy(fileEntry->longNameEntry.nameStart,longName,10);
		memcpy(fileEntry->longNameEntry.nameMiddle,&longName[5],12);
		memcpy(fileEntry->longNameEntry.nameEnd,&longName[11],4);
		strncpy(fullShortName,shortName,8);
		strncpy(fullShortName+8,extension,3);
		fileEntry->longNameEntry.checksum=lfn_checksum(fullShortName);
		fileEntry->hasLongNameEntry=1;
	}
	return 0;
}

int fat_shortNameExists(char * nameWithoutExtension,t_fat_file_entry * dir){
	t_fat_file_list *p, *dirList;
	p = dirList = fat_getDirectoryListing(dir);
	while(p!=NULL){
		if(memcmp(nameWithoutExtension,p->fileEntry.dataEntry.name,8)==0)
			return 1;
		p=p->next;
	}
	return 0;
}

int fat_generateShortName(const char * newName, t_fat_file_entry * dir, char * shortName){
	if (strlen(newName)<= 8){
		memset(shortName,' ',8);
		memcpy(shortName,newName,strlen(newName));
		shortName[8]='\0';
		return 0;
	}else{
		memcpy(shortName,newName,6);
		shortName[6]='˜';
		int i,valid=0;
		for (i=1;i<10;i++){
			shortName[7]='0'+i;
			if(!fat_shortNameExists(shortName,dir)){
				valid=1;
				break;
			}
		}
		assert(valid==1);
		shortName[8]='\0';
	}

	stringToUpper(shortName);
	return 0;
}

int needsLongName(const char * name){ //TODO: Evaluar si es necesario
	return 1;
}

void stringToUpper(char * string){
	int i;
	for (i=0;i<strlen(string);i++)
		string[i]=toupper(string[i]);
}

uint8_t lfn_checksum(const unsigned char *pFcbName){
	int i;
	unsigned char sum=0;

	for (i=11; i; i--)
			sum = ((sum & 1) << 7) + (sum >> 1) + *pFcbName++;
	return sum;
}

uint32_t fat_getFreeClusterCount(){
	uint32_t i,count=0,totalClusters=bootSector.totalSectors32/bootSector.sectorPerCluster;
	for(i=0;i<totalClusters;i++){
		if(fat_getNextCluster(i)==FAT_FREE_CLUSTER) count++;
	}
	return count;
}
uint32_t fat_getUsedClusterCount(){
	uint32_t i,count=0,totalClusters=bootSector.totalSectors32/bootSector.sectorPerCluster;
	for(i=0;i<totalClusters;i++){
		if(fat_getNextCluster(i)!=FAT_FREE_CLUSTER) count++;
	}
	return count;
}

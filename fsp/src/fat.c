/*
 * fat.c
 *
 *  Created on: Sep 2, 2011
 *      Author: nico
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include "fatTypes.h"
#include "fatAddressing.h"
#include "fat.h"
#include "fatHelpers.h"
#include "disk.h"
#include "ctype.h"
#include "sys/stat.h"
#include <assert.h>
#include "common/utils/utils.h"
#include "common/collections/list.h"
#include <math.h>
#include "common/utils/config.h"
#include <signal.h>


t_fat_bootsector bootSector;
uint32_t * fatTable;
extern config_fsp * config;
t_list *filesCache;
volatile sig_atomic_t dumping=0;
sem_t fatTableSemaphore, fatClusterChainSem;

void fat_initialize(){
	disk_initialize();
	sem_init(&fatTableSemaphore,0,1);
	sem_init(&fatClusterChainSem,0,1);

	filesCache = collection_list_create();
	signal(SIGUSR1,fat_signalHandler);
	bootSector = fat_readBootSector();
	fat_loadFAT();
}

void fat_loadFAT(){
	uint32_t i,totalClusters=bootSector.totalSectors32/bootSector.sectorPerCluster;
	t_block block;
	uint32_t entriesPerBlock =FAT_BLOCK_SIZE/FAT_FAT_ENTRY_SIZE;
	fatTable = (uint32_t *) malloc(totalClusters * FAT_FAT_ENTRY_SIZE);
	for (i=0;i<totalClusters/ (entriesPerBlock);i++){
		fat_addressing_readBlocks(fat_getFATFirstBlock()+i,1,block);
		memcpy(fatTable+i*entriesPerBlock,&block,FAT_BLOCK_SIZE);
	}
	return;
}

t_fat_bootsector fat_readBootSector(){
	t_fat_bootsector bs;
	t_block block;
	if (fat_addressing_readBlocks(0,1,block)){
		memcpy(&bs,&block,sizeof(t_fat_bootsector));
	}
	return bs;
}

t_fat_file_list * fat_getFileListFromDirectoryCluster(uint32_t clusterN){
	t_fat_file_data_entry tempDataEntry;
	t_fat_file_list * directory = NULL;
	t_fat_file_list * p;
	t_fat_file_list * pAnt=NULL;
	t_cluster cluster;
	uint32_t inEntry=0;
	uint32_t i;
	while (1){
		//dataCluster=fat_dataClusterToDiskCluster(clusterN);
		fat_addressing_readCluster(clusterN,cluster);
		for(i =0; i<sizeof(t_cluster);i+=sizeof(t_fat_file_data_entry)){
				//Asumo que antes de la entrada posta tengo n de nombre largo, me quedo con la ultima nomas
				//TODO: Opcional: Implementar LFN como corresponde (extra TP, copado para hacer testing)
				memcpy(&tempDataEntry,cluster+i,sizeof(t_fat_file_data_entry));

				if (tempDataEntry.name[0]==0){ //La primera entrada despues de de la ultima entrada con datos empiesa con 0, me avisa que se termino el listado.
					return directory;
				}
				if(tempDataEntry.name[0]==0xE5){
					continue;
				}
				if (!inEntry){
					p=(t_fat_file_list * )malloc(sizeof(t_fat_file_list));
					memset(&p->fileEntry,0,sizeof(t_fat_file_entry));
					inEntry=1;
				}
				if (tempDataEntry.attributes==0x0F) {// Es entrada de nombre largo
					memcpy(&(p->fileEntry.longNameEntry),&tempDataEntry,sizeof(t_fat_long_name_entry));
					p->fileEntry.hasLongNameEntry=1;
				}else{//Es entrada de archivo
					memcpy(&(p->fileEntry.dataEntry),&tempDataEntry,sizeof(t_fat_file_data_entry));
					inEntry=0;
					if(pAnt!=NULL){
						pAnt->next=p;
					}else{
						directory=p;
					}
					p->next=NULL;
					pAnt=p;

				}
		}
		clusterN=fat_getNextCluster(clusterN);
	}
	return directory;
}

void fat_destroyFileList(t_fat_file_list * fileList){
	t_fat_file_list * p;
	while(fileList!=NULL){
		p=fileList->next;
		free(fileList);
		fileList=p;
	}
}

t_fat_file_list * fat_getRootDirectory(){
	return fat_getFileListFromDirectoryCluster(2);
}

int fat_getFileFromPath(const char * path,t_fat_file_entry * rtn){
	t_fat_file_list * dir = fat_getRootDirectory();
	t_fat_file_entry fileEntry;
	t_fat_file_entry * found;
	if(strcmp(path,"/")==0){
		fat_destroyFileList(dir);
		fat_getRootDirectoryEntry(rtn);
		return 1;
	}
	char ** subpaths;
	subpaths= string_split2((char *)path,'/');
	int i=1;
	while(subpaths[i]!=NULL){
		found=fat_findInDir(dir,subpaths[i]);
		if (found==NULL){
			fat_destroyFileList(dir);
			freeArrayofPointersToStrings(subpaths,255);
			return 0;
		}
		if((found->dataEntry.attributes!=0x10) && (subpaths[i+1]!=NULL)){
			fat_destroyFileList(dir);
			freeArrayofPointersToStrings(subpaths,255);
			return 0;
		}
		memcpy(&fileEntry,found,sizeof(t_fat_file_entry));
		fat_destroyFileList(dir);
		if(subpaths[i+1]==NULL){
			memcpy(rtn,&fileEntry,sizeof(t_fat_file_entry));
			freeArrayofPointersToStrings(subpaths,255);
			return 1;
		}else{
			dir = fat_getDirectoryListing(&fileEntry);
		}
		i++;
	}
	freeArrayofPointersToStrings(subpaths,255);
	return 1;
}

void freeArrayofPointersToStrings(char ** caca,size_t arraySize){
	int i=0;
	while(caca[i]!=NULL){
		free(caca[i]);
		i++;
	}
	free(caca);
}
t_fat_file_list * fat_getDirectoryListing(t_fat_file_entry * fileEntry){
	uint32_t clusterN;
	if (fileEntry==NULL)
		return NULL;
	clusterN=fat_getEntryFirstCluster(&fileEntry->dataEntry);
	return fat_getFileListFromDirectoryCluster(clusterN);
}


t_fat_file_data_entry * fat_getNextEntry(const t_fat_file_list * dir){
	//TODO: Evaluar si me simplifica la semantica
	return 0;
}

t_stat fat_statFile(t_fat_file_entry * file){
	t_stat fileStat;
	memset(&fileStat,0,sizeof(t_stat));
	if (file->dataEntry.attributes==0x10){
		fileStat.st_mode = S_IFDIR | 0755;
		fileStat.st_nlink = 2;
		fileStat.st_size = FAT_CLUSTER_SIZE;
	}else if (file->dataEntry.attributes==0x20){
		fileStat.st_mode = S_IFREG | 0666;
		fileStat.st_nlink = 1;
		fileStat.st_size=file->dataEntry.fileSize;
	}
	return fileStat;
}
uint32_t fat_advanceNClusters(uint32_t clusterStart, uint32_t offset){
	uint32_t i;
	for (i=0;i< offset;i++){
		clusterStart=fat_getNextCluster(clusterStart);
		if (clusterStart==FAT_LAST_CLUSTER) return 0;
	}
	return clusterStart;
}

int fat_readFileContents(t_fat_file_entry * fileEntry,size_t size, off_t offset, char * buf,const char * path){
	//HERE BE DRAGONS
	t_cluster cluster;
	uint32_t dataCluster, sizeToRead;
	uint32_t i,offsetInsideCluster,leftToRead;
	uint32_t positionInBuffer,positionInFile;
	uint32_t clustersInRead;
	uint32_t thisReadSize;

	dataCluster = fat_getEntryFirstCluster(&fileEntry->dataEntry);
	if(offset > fileEntry->dataEntry.fileSize) // si me pide leer desde despues del fin de archivo devuelvo 0
		return 0;
	if (offset + size > fileEntry->dataEntry.fileSize){ // si me pide leer desde antes del EOF hasta despues, leo hasta el EOF y devuelvo la cantidad que lei
		sizeToRead = fileEntry->dataEntry.fileSize - offset;
	}else{
		sizeToRead = size;
	}
	clustersInRead = ceil(((float)offset+sizeToRead)/FAT_CLUSTER_SIZE)-((float)offset/FAT_CLUSTER_SIZE); //cuantos cluster me insume la lectura
	// Me ubico en el primer cluster de la lectura
	dataCluster=fat_advanceNClusters(dataCluster,offset/FAT_CLUSTER_SIZE);
	leftToRead = sizeToRead;
	positionInFile= offset;
	positionInBuffer=0;
	for (i=0;i<clustersInRead;i++){
		//diskCluster=fat_dataClusterToDiskCluster(dataCluster);

		fat_file_readCluster(path, dataCluster,cluster);

		offsetInsideCluster = positionInFile % FAT_CLUSTER_SIZE;
		if(leftToRead > FAT_CLUSTER_SIZE - offsetInsideCluster){
			thisReadSize=FAT_CLUSTER_SIZE - offsetInsideCluster;
		}else{
			thisReadSize=leftToRead;
		}
		memcpy(&buf[positionInBuffer],&cluster[offsetInsideCluster],thisReadSize);
		positionInFile+=thisReadSize;
		positionInBuffer+=thisReadSize;
		leftToRead-=thisReadSize;
		dataCluster=fat_getNextCluster(dataCluster);
	}
	return sizeToRead;


}

void fat_getName (t_fat_file_entry * fileEntry, char * buff){
	uint16_t longNameUTF16[14];
	char longName[14]="";
	int i;
	if (fileEntry->hasLongNameEntry==1){
		memcpy(longNameUTF16,&fileEntry->longNameEntry.nameStart,10);
		memcpy(longNameUTF16+5,&fileEntry->longNameEntry.nameMiddle,12);
		memcpy(longNameUTF16+11,&fileEntry->longNameEntry.nameEnd,4);
		longNameUTF16[13]=0x0000;
		for(i=0;i<14;i++){
			if(longNameUTF16[i]==0x0000) break;
		}
		unicode_utf16_to_utf8_inbuffer(longNameUTF16,i,longName,NULL);
	}else{
		char name[9];
		char ext[4];
		char * space;
		strncpy(name,(char *)fileEntry->dataEntry.name,8);
		name[8]='\0';
		strncpy(ext,(char *)fileEntry->dataEntry.extension,3);
		ext[3]='\0';
		if ((space=strchr(name,' '))!=NULL)
			*space='\0';
		if ((space=strchr(ext,' '))!=NULL)
			*space='\0';
		if (strlen(ext)>0)
			sprintf(longName,"%s.%s",name,ext);
		else
			sprintf(longName,"%s",name);
	}
	strncpy(buff,longName,14);
	return;
}

int fat_addFreeClusterToChain(uint32_t lastClusterOfChain){
	t_cluster cluster;
	memset(cluster,0,FAT_CLUSTER_SIZE);
	uint32_t freeCluster=fat_getNextFreeCluster(0);
	assert(fat_getNextCluster(lastClusterOfChain)==FAT_LAST_CLUSTER);
	fat_fat_setValue(lastClusterOfChain,freeCluster);
	fat_fat_setValue(freeCluster,FAT_LAST_CLUSTER);
	fat_addressing_writeCluster(freeCluster,cluster);
	return 0;
}
int fat_removeLastClusterFromFile(t_fat_file_entry * file){
	sem_wait(&fatClusterChainSem);
	uint32_t lastCluster,theOneBefore;
	lastCluster = fat_getFileLastCluster(file);
	if(lastCluster!=fat_getEntryFirstCluster(&file->dataEntry)){
		theOneBefore = fat_getClusterPointingTo(lastCluster,fat_getEntryFirstCluster(&file->dataEntry));
		fat_fat_setValue(lastCluster,FAT_FREE_CLUSTER);
		fat_fat_setValue(theOneBefore,FAT_LAST_CLUSTER);
	}else{
		fat_fat_setValue(lastCluster,FAT_FREE_CLUSTER);
		fat_setEntryFirstCluster(0,&file->dataEntry);
	}
	sem_post(&fatClusterChainSem);
	return 0;
}

int fat_addClusterToFile(t_fat_file_entry * file){
	sem_wait(&fatClusterChainSem);
	if(fat_getEntryFirstCluster(&file->dataEntry)!=0){
		uint32_t lastCluster = fat_getFileLastCluster(file);
		fat_addFreeClusterToChain(lastCluster);
	}else{
		t_cluster cluster;
		memset(cluster,0,FAT_CLUSTER_SIZE);
		uint32_t firstCluster=fat_getNextFreeCluster(0);
		fat_fat_setValue(firstCluster,FAT_LAST_CLUSTER);
		fat_setEntryFirstCluster(firstCluster,&file->dataEntry);
		fat_addressing_writeCluster(firstCluster,cluster);
	}
	sem_post(&fatClusterChainSem);
	return 0;
}

int fat_truncate(const char * path,off_t newSize){
	t_fat_file_entry file;
	int32_t clustersDelta;
	uint32_t i,clustersNeeded,clustersInFile;
	fat_getFileFromPath(path,&file);
	clustersNeeded =ceil((float)newSize / FAT_CLUSTER_SIZE);
	clustersInFile = fat_getClusterCount(&file.dataEntry);
	clustersDelta= clustersNeeded - clustersInFile;
	if (clustersDelta<0){//lo tengo que achicar
		for (i=0;i<abs(clustersDelta);i++){
			fat_removeLastClusterFromFile(&file);
		}
	}else{
		for (i=0;i<abs(clustersDelta);i++){
			fat_addClusterToFile(&file);
		}
	}
	file.dataEntry.fileSize = newSize;
	fat_setFileEntry(path,&file);
	return 0;
}

int fat_setFileEntry(const char *path, t_fat_file_entry * fileEntry){
	uint32_t i;
	t_fat_file_entry directory, tempEntry;
	t_fat_file_data_entry tempDataEntry;
	t_cluster cluster;
	uint32_t clusterN;
	int inEntry=0;
	uint32_t entryStart;
	assert(strcmp(path,"/")!=0);
	char directoryPath[255];
	char fileName[15],anotherName[15];
	splitPathName(path,directoryPath,fileName);
	fat_getFileFromPath(directoryPath,&directory);
	clusterN =fat_getEntryFirstCluster(&directory.dataEntry);
	fat_addressing_readCluster(clusterN,cluster);

	for(i=0;i<FAT_CLUSTER_SIZE;i+=sizeof(t_fat_file_data_entry)){
		memcpy(&tempDataEntry,cluster+i,sizeof(t_fat_file_data_entry));

		if (tempDataEntry.name[0]==0){ //La primera entrada despues de de la ultima entrada con datos empiesa con 0, me avisa que se termino el listado.
			return 0;
		}
		if(tempDataEntry.name[0]==0xE5){
			continue;
		}
		if (!inEntry){
			memset(&tempEntry,0,sizeof(t_fat_file_entry));
			entryStart = i;
			inEntry=1;
		}
		if (tempDataEntry.attributes==0x0F) {// Es entrada de nombre largo
			assert(tempEntry.hasLongNameEntry==0);
			memcpy(&(tempEntry.longNameEntry),&tempDataEntry,sizeof(t_fat_long_name_entry));
			tempEntry.hasLongNameEntry=1;
		}else{// Entrada de data
			memcpy(&(tempEntry.dataEntry),&tempDataEntry,sizeof(t_fat_file_data_entry));
			fat_getName(&tempEntry,anotherName);
			if(strcmp(anotherName,fileName)==0){
				memcpy(&cluster[entryStart],&fileEntry->longNameEntry,sizeof(t_fat_long_name_entry));
				memcpy(&cluster[entryStart+sizeof(t_fat_long_name_entry)],&fileEntry->dataEntry,sizeof(t_fat_file_data_entry));
				break;
			}
			inEntry=0;
		}
	}
	fat_addressing_writeCluster(clusterN,cluster);
	return 1;
}

void fat_getRootDirectoryEntry(t_fat_file_entry * rootDirectoryEntry){
	memset(rootDirectoryEntry,0,sizeof(t_fat_file_entry));
	rootDirectoryEntry->hasLongNameEntry=0;
	rootDirectoryEntry->dataEntry.attributes=0x10;
	rootDirectoryEntry->dataEntry.fileSize=FAT_CLUSTER_SIZE;
	rootDirectoryEntry->dataEntry.firstClusterHigh=0;
	rootDirectoryEntry->dataEntry.firstClusterLow=2;
}

int fat_move(const char * from,const char * to){
	char newDirectory[255], newName[15];
	t_fat_file_entry fileEntry,fileEntryDeleted,destinationDir;
	fat_getFileFromPath(from,&fileEntry);
	memcpy(&fileEntryDeleted,&fileEntry,sizeof(t_fat_file_entry));
	fileEntryDeleted.dataEntry.name[0]=0xE5;
	fileEntryDeleted.longNameEntry.sequenceN=0xE5;
	fat_setFileEntry(from,&fileEntryDeleted);
	splitPathName(to,newDirectory,newName);
	fat_getFileFromPath(newDirectory,&destinationDir);
	fat_renameFile(newName,&destinationDir,&fileEntry);
	fat_addEntry(newDirectory,fileEntry);
	return 0;
}

uint32_t fat_findFreeEntries(t_cluster * cluster,size_t numberOfEntries){
	t_fat_file_data_entry tempDataEntry;
	uint32_t i, free_start=0,free_size=0, inFreeSpace=0;
	for(i=0;i<FAT_CLUSTER_SIZE;i+=sizeof(t_fat_file_data_entry)){
		memcpy(&tempDataEntry,*cluster+i,sizeof(t_fat_file_data_entry));

		if ( (tempDataEntry.name[0]==0) || (tempDataEntry.name[0]==0xE5)){ //La primera entrada despues de de la ultima entrada con datos empiesa con 0, me avisa que se termino el listado.
			if(!inFreeSpace){
				inFreeSpace=1;
				free_start=i;
			}
			free_size+=1;
		}else{
			inFreeSpace=0;
		}
		if(free_size==numberOfEntries){
			return free_start;
		}
	}
	return -1;
}

int fat_addEntry(const char * directoryPath, t_fat_file_entry fileEntry){
	t_fat_file_entry dirEntry;
	t_cluster cluster;
	uint32_t dataCluster, freeSpaceStart;
	fat_getFileFromPath(directoryPath,&dirEntry);
	dataCluster = fat_getEntryFirstCluster(&dirEntry.dataEntry);
	fat_addressing_readCluster(dataCluster,cluster);
	while((freeSpaceStart = fat_findFreeEntries(&cluster,2))==-1){
		dataCluster=fat_getNextCluster(dataCluster);
		if (dataCluster==FAT_LAST_CLUSTER){
			fat_addClusterToFile(&dirEntry);
			dataCluster=fat_getFileLastCluster(&dirEntry);
			memset(&cluster,0,FAT_CLUSTER_SIZE);
		}else{
			fat_addressing_readCluster(dataCluster,cluster);
		}
	}
	memcpy(&cluster[freeSpaceStart],&fileEntry.longNameEntry,sizeof(t_fat_long_name_entry));
	memcpy(&cluster[freeSpaceStart + sizeof(t_fat_long_name_entry)],&fileEntry.dataEntry,sizeof(t_fat_file_data_entry));
	fat_addressing_writeCluster(dataCluster,cluster);
	return 0;
}


int fat_write(const char *path, const char *buf, size_t size, off_t offset){
	t_cluster cluster;
	uint32_t dataCluster, sizeToWrite;
	uint32_t i,offsetInsideCluster,leftToWrite;
	uint32_t positionInBuffer,positionInFile;
	uint32_t clustersInWrite;
	uint32_t thisWriteSize;
	t_fat_file_entry fileEntry;
	fat_getFileFromPath(path,&fileEntry);


	if(offset +size > fileEntry.dataEntry.fileSize){ // si me pide escribir mas alla del tamaño maximo, lo trunco al tamaño que necesite
		fat_truncate(path,offset+size);
		fat_getFileFromPath(path,&fileEntry);
	}
	dataCluster = fat_getEntryFirstCluster(&fileEntry.dataEntry);
	sizeToWrite = size;
	clustersInWrite = ceil((((float)offset+sizeToWrite)/FAT_CLUSTER_SIZE)-((float)offset/FAT_CLUSTER_SIZE)); //cuantos cluster me insume la escritura
	// Me ubico en el primer cluster de la escritura
	dataCluster=fat_advanceNClusters(dataCluster,offset/FAT_CLUSTER_SIZE);
	leftToWrite = sizeToWrite;
	positionInFile= offset;
	positionInBuffer=0;
	for (i=0;i<clustersInWrite;i++){

		fat_file_readCluster(path,dataCluster,cluster);

		offsetInsideCluster = positionInFile % FAT_CLUSTER_SIZE;
		if(leftToWrite > FAT_CLUSTER_SIZE - offsetInsideCluster){
			thisWriteSize=FAT_CLUSTER_SIZE - offsetInsideCluster;
		}else{
			thisWriteSize=leftToWrite;
		}
		memcpy(&cluster[offsetInsideCluster],&buf[positionInBuffer],thisWriteSize);
		fat_file_writeCluster(path,dataCluster,cluster);
		positionInFile+=thisWriteSize;
		positionInBuffer+=thisWriteSize;
		leftToWrite-=thisWriteSize;
		dataCluster=fat_getNextCluster(dataCluster);
	}
	return sizeToWrite;

	return 0;
}

int fat_mkdir(const char * path){
	char parent[255], name[15];
	t_fat_file_entry fileEntry,parentEntry;
	memset(&fileEntry,0,sizeof(fileEntry));
	splitPathName(path,parent,name);
	fat_getFileFromPath(parent,&parentEntry);
	fileEntry.dataEntry.attributes=0x10; //tengo que setarlo antes del rename
	fileEntry.dataEntry.fileSize= FAT_CLUSTER_SIZE;
	fat_renameFile(name,&parentEntry,&fileEntry);
	fileEntry.longNameEntry.sequenceN= 0x41; //Primera y ultima (0x01+0x40);
	fileEntry.longNameEntry.attributes=0x0F;
	fat_setEntryFirstCluster(0,&fileEntry.dataEntry);
	fat_addClusterToFile(&fileEntry);
	fat_addEntry(parent,fileEntry);
	return 0;
}

int fat_mkFile(const char * path){
	char parent[255], name[15];
	t_fat_file_entry fileEntry,parentEntry;
	memset(&fileEntry,0,sizeof(fileEntry));
	splitPathName(path,parent,name);
	fat_getFileFromPath(parent,&parentEntry);
	fileEntry.dataEntry.attributes=0x20; //tengo que setarlo antes del rename
	fileEntry.dataEntry.fileSize=0;
	fat_setEntryFirstCluster(0,&fileEntry.dataEntry);
	fileEntry.hasLongNameEntry=1;
	fat_renameFile(name,&parentEntry,&fileEntry);
	fileEntry.longNameEntry.sequenceN= 0x41; //Primera y ultima (0x01+0x40);
	fileEntry.longNameEntry.attributes=0x0F;
	fat_addEntry(parent,fileEntry);
	return 0;
}

int fat_unlink(const char * path){
	t_fat_file_entry fileEntry;
	fat_getFileFromPath(path,&fileEntry);
	assert(fileEntry.dataEntry.attributes==0x20);
	fat_truncate(path,0);
	fileEntry.dataEntry.name[0]=0xE5;
	if(fileEntry.hasLongNameEntry)
		fileEntry.longNameEntry.sequenceN=0xE5;
	fat_setFileEntry(path,&fileEntry);
	return 0;
}

int fat_rmdir(const char * path){
	t_fat_file_entry fileEntry;
	fat_getFileFromPath(path,&fileEntry);
	assert(fileEntry.dataEntry.attributes==0x10);
	fat_truncate(path,0);
	fileEntry.dataEntry.name[0]=0xE5;
	if(fileEntry.hasLongNameEntry)
		fileEntry.longNameEntry.sequenceN=0xE5;
	fat_setFileEntry(path,&fileEntry);
	return 0;
}


//**************************************** Cache Functions *************************************************

void fat_createFileCache(const char * path){

	int findFromPath(void * node){
		t_fat_cache_list * node2 =(t_fat_cache_list *)node;
		if (strcmp(path,node2->path)==0)return 1;
		return 0;

	}
	t_fat_cache_list * fileNode;
	if((fileNode = collection_list_find(filesCache,findFromPath))!= NULL){
		fileNode->openCount++;
		return;
	}

	if (config->sizeCache==0)
		return;
	int32_t clustersCached = config->sizeCache / FAT_CLUSTER_SIZE;
	t_fat_file_cache * cache = (t_fat_file_cache *) malloc (sizeof(t_fat_file_cache) * clustersCached);
	memset(cache,0xFF,sizeof(t_fat_file_cache) * clustersCached);

	fileNode = (t_fat_cache_list *) malloc (sizeof(t_fat_cache_list));
	strcpy(fileNode->path,path);
	fileNode->openCount = 1;
	fileNode->clusterStart= 0xFFFFFFFF;
	fileNode->cache=cache;
	sem_init(&fileNode->inUse,0,1);
	collection_list_add(filesCache,fileNode);
}

void fat_destroyFileCache(const char * path){
	int findFromPath(void * node){
		t_fat_cache_list * node2 =(t_fat_cache_list *)node;
		if (strcmp(path,node2->path)==0)return 1;
		return 0;

	}
	void freeCacheNode(void * node){
		t_fat_cache_list * node2 =(t_fat_cache_list *)node;
		int32_t i,clustersCached = config->sizeCache / FAT_CLUSTER_SIZE;
		for(i=0;i<clustersCached;i++){
			if(node2->cache[i].clusterNumber!=0xFFFFFF){
				fat_addressing_writeCluster(node2->cache[i].clusterNumber,node2->cache[i].data);
			}
		}

		free(node2->cache);
	}

	if (config->sizeCache==0)
		return;

	t_fat_cache_list * fileNode = collection_list_find(filesCache,findFromPath);
	assert(fileNode!=NULL);
	if (--fileNode->openCount > 0) return;

	collection_list_removeByClosure(filesCache,findFromPath,freeCacheNode);
}

void fat_fileCacheLoad(t_fat_file_cache * cache,uint32_t clusterNumber){
	uint32_t i;
	if (config->sizeCache==0)
		return;
	int32_t clustersCached = config->sizeCache / FAT_CLUSTER_SIZE;
	for(i=0;i<clustersCached;i++){
		if(cache[i].clusterNumber!=0xFFFFFFFF){
			fat_addressing_writeCluster(cache[i].clusterNumber,cache[i].data);
		}
		cache[i].clusterNumber = clusterNumber + i;
		fat_addressing_readCluster(clusterNumber + i,cache[i].data);
	}
}

void fat_fileCacheFlush(const char * path){
	int findFromPath(void * node){
		t_fat_cache_list * node2 =(t_fat_cache_list *)node;
		if (strcmp(path,node2->path)==0)return 1;
		return 0;

	}

	if(config->sizeCache==0){
		return;
	}

	int32_t i,clustersCached = config->sizeCache / FAT_CLUSTER_SIZE;
	t_fat_cache_list * fileNode = collection_list_find(filesCache,findFromPath);
	sem_wait(&fileNode->inUse);
	for(i=0;i<clustersCached;i++){
		if(fileNode->cache[i].clusterNumber!=0xFFFFFFFF){
			fat_addressing_writeCluster(fileNode->cache[i].clusterNumber,fileNode->cache[i].data);
		}
	}
	sem_post(&fileNode->inUse);
}
void fat_file_readCluster(const char * path,uint32_t clusterNumber,t_cluster cluster){
	int findFromPath(void * node){
		t_fat_cache_list * node2 =(t_fat_cache_list *)node;
		if (strcmp(path,node2->path)==0)return 1;
		return 0;

	}
	if(config->sizeCache==0){
		fat_addressing_readCluster(clusterNumber, cluster);
		return;
	}
	t_fat_cache_list * fileNode = collection_list_find(filesCache,findFromPath);
	sem_wait(&fileNode->inUse);
	uint32_t cacheEntry = clusterNumber - fileNode->clusterStart;
	int32_t clustersCached = config->sizeCache / FAT_CLUSTER_SIZE;
	if ( cacheEntry >= 0 && cacheEntry < clustersCached){
		memcpy(cluster,fileNode->cache[cacheEntry].data,FAT_CLUSTER_SIZE);
		sem_post(&fileNode->inUse);
		return;
	}
	fat_fileCacheLoad(fileNode->cache,clusterNumber);
	fileNode->clusterStart=clusterNumber;
	memcpy(cluster,fileNode->cache[0].data,FAT_CLUSTER_SIZE);
	sem_post(&fileNode->inUse);
	return;
}

void fat_file_writeCluster(const char * path,uint32_t clusterNumber,t_cluster cluster){
	int findFromPath(void * node){
		t_fat_cache_list * node2 =(t_fat_cache_list *)node;
		if (strcmp(path,node2->path)==0)return 1;
		return 0;

	}

	if(config->sizeCache==0){
		fat_addressing_writeCluster(clusterNumber, cluster);
		return;
	}
	int32_t clustersCached = config->sizeCache / FAT_CLUSTER_SIZE;
	t_fat_cache_list * fileNode = collection_list_find(filesCache,findFromPath);
	sem_wait(&fileNode->inUse);
	uint32_t cacheEntry = clusterNumber - fileNode->clusterStart;
	if ( cacheEntry >= 0 && cacheEntry < clustersCached){
		memcpy(fileNode->cache[cacheEntry].data,cluster,FAT_CLUSTER_SIZE);
		sem_post(&fileNode->inUse);
		return;
	}
	fat_fileCacheLoad(fileNode->cache,clusterNumber);
	fileNode->clusterStart=clusterNumber;
	memcpy(fileNode->cache[0].data,cluster,FAT_CLUSTER_SIZE);
	sem_post(&fileNode->inUse);
	return;
}

int fat_signalHandler(int signum){
	if (dumping ==1){
		perror("ya estoy dumpeando, no rompas las... Ejem, intente nuevamente mas tarde");
	}else{
		dumping=1;
		FILE * f = fopen("cache_dump.txt","ab");
		int32_t i,clustersCached = config->sizeCache / FAT_CLUSTER_SIZE;
		void printCacheContents(void * node){
			t_fat_cache_list * node2 =(t_fat_cache_list *)node;
			fprintf(f,"-----------------------------------------\n");
			//TODO: Timestamp
			fprintf(f,"Archivo: %s\n",node2->path);
			fprintf(f,"Tamaño de Bloque de Cache: %d\n",FAT_CLUSTER_SIZE);
			fprintf(f,"Cantidad de Bloques de Cache: %d\n",clustersCached);
			for (i=0;i<clustersCached;i++){
				fprintf(f,"Contenido de Bloque de Cache %d:\n",i);
				fprintf(f,"%s\n",node2->cache[i].data);
			};
			return;
		}
		dumping=0;
		collection_list_iterator(filesCache,printCacheContents);
		fclose(f);
	}
	return dumping;
}

void fat_cleanup(){
	disk_cleanup();
	free(fatTable);
}


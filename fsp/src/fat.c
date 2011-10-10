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
#include "utils.h"
#include <math.h>


t_fat_bootsector bootSector;
t_fat_config fatConfig;
uint32_t * fatTable;

int main2(){

	char ** subpaths=string_split2("/aqwe/bxcv/cjhdb/d123.jpg",'/');
	uint32_t i=0;
	while(subpaths[i]!=NULL){
		printf("%s\n",subpaths[i]);
		i++;
	}
	return 0;
	fat_initialize();
	t_fat_file_list * dir, *p;
	//t_fat_file_entry file;;
	char fileName[15];
	//fat_getNextCluster(44884);
	/*if (fat_getFileFromPath("/UNDIR/DOSDIR",&fileEntry)){
		dir = fat_getDirectoryListing(&fileEntry);
	}else{
		dir = NULL;
	}*/
	for(i=0; i<=100;i++){
		dir = fat_getRootDirectory();
		p=dir;
		while (p!=NULL){
			switch (p->fileEntry.dataEntry.name[0]) {
			case 0x00:
				//Vacia, no hay mas
				return 0;
			case 0x2E:
				puts("Entrada de punto!");
				break;
			case 0xE5:
				puts("Entrada Borrada!");
				break;
			default:
				fat_getName(&p->fileEntry,fileName);
				printf("nombre:%s\n\n",fileName);
				fat_statFile(&p->fileEntry);
				char * content = (char *) malloc(p->fileEntry.dataEntry.fileSize);
				fat_readFileContents(&p->fileEntry,p->fileEntry.dataEntry.fileSize,0,content);
				free(content);
				break;
			}
			p=p->next;

		}
		fat_destroyFileList(dir);
	}
	return 0;

}

void fat_initialize(){
	disk_initialize();
	bootSector = fat_readBootSector();
	//TODO: Setearlo desde config
	strcpy(fatConfig.diskIp,"127.0.0.1");
	fatConfig.diskPort=5678;
	fatConfig.bindPort=5679;
	fatConfig.cacheSizeInClusters=8;
	fat_loadFAT();
}

void fat_loadFAT(){
	uint32_t i,totalClusters=bootSector.totalSectors32/bootSector.sectorPerCluster;
	t_cluster cluster;
	uint32_t entriesPerCluster =FAT_CLUSTER_SIZE/FAT_FAT_ENTRY_SIZE;
	fatTable = (uint32_t *) malloc(totalClusters * FAT_FAT_ENTRY_SIZE);
	for (i=0;i<totalClusters/ (entriesPerCluster);i++){
		fat_addressing_readCluster(fat_getFATFirstCluster()+i,&cluster);
		memcpy(fatTable+i*entriesPerCluster,&cluster,FAT_CLUSTER_SIZE);
	}
	return;
}

t_fat_bootsector fat_readBootSector(){
	t_fat_bootsector bs;
	t_sector cluster;
	if (disk_readSector(0,&cluster)){
		memcpy(&bs,&cluster,sizeof(t_fat_bootsector));
	}
	return bs;
}

t_fat_file_list * fat_getFileListFromDirectoryCluster(uint32_t clusterN){
	t_fat_file_data_entry tempDataEntry;
	t_fat_file_list * directory = NULL;
	t_fat_file_list * p;
	t_fat_file_list * pAnt=NULL;
	t_cluster cluster;
	int inEntry=0;
	int i;
	while (1){
		fat_addressing_readCluster(clusterN,&cluster);
		for(i =0; i<sizeof(t_cluster);i+=sizeof(t_fat_file_data_entry)){
				//TODO:Semantica mas linda
				//Asumo que antes de la entrada posta tengo n de nombre largo, me quedo con la ultima nomas
				//TODO: Implementar LNF como corresponde (extra TP, copado para hacer testing)
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
	return fat_getFileListFromDirectoryCluster(fat_getRootDirectoryFirstCluster());
}

int fat_getFileFromPath(const char * path,t_fat_file_entry * rtn){
	t_fat_file_list * dir = fat_getRootDirectory();
	t_fat_file_entry fileEntry;
	t_fat_file_entry * found;
	if(strcmp(path,"/")==0){
		fat_destroyFileList(dir);
		return -1;
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
	clusterN=fat_dataClusterToDiskCluster(fat_getEntryFirstCluster(&fileEntry->dataEntry));
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
		fileStat.st_mode = S_IFREG | 0444;
		fileStat.st_nlink = 1;
		fileStat.st_size=file->dataEntry.fileSize;
	}
	return fileStat;
}

int fat_readFileContents(t_fat_file_entry * fileEntry,size_t size, off_t offset, char * buf){
	//HERE BE DRAGONS
	t_cluster cluster;
	uint32_t dataCluster,diskCluster, sizeToRead;
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
	for (i=0;i< offset/FAT_CLUSTER_SIZE;i++){
		dataCluster=fat_getNextCluster(dataCluster);
		if (dataCluster==FAT_LAST_CLUSTER) return 0;
	}
	leftToRead = sizeToRead;
	positionInFile= offset;
	positionInBuffer=0;
	for (i=0;i<clustersInRead;i++){
		diskCluster=fat_dataClusterToDiskCluster(dataCluster);
		fat_addressing_readCluster(diskCluster,&cluster);
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
	char longName[14];
	if (fileEntry->hasLongNameEntry==1){
		memcpy(longNameUTF16,&fileEntry->longNameEntry.nameStart,10);
		memcpy(longNameUTF16+5,&fileEntry->longNameEntry.nameMiddle,12);
		memcpy(longNameUTF16+11,&fileEntry->longNameEntry.nameEnd,4);
		longNameUTF16[13]=0x0000;
		unicode_utf16_to_utf8_inbuffer(longNameUTF16,13,longName,NULL);
	}else{
		char name[8];
		char ext[3];
		char * space;
		strncpy(name,(char *)fileEntry->dataEntry.name,8);
		strncpy(ext,(char *)fileEntry->dataEntry.extension,3);
		if ((space=strchr(name,' '))!=NULL)
			*space='\0';
		if ((space=strchr(ext,' '))!=NULL)
			*space='\0';
		if (strlen(ext)>0)
			sprintf(longName,"%s.%s",name,ext);
		else
			sprintf(longName,"%s",name);
	}
	strncpy(buff,longName,13);
	return;
}

int fat_addFreeClusterToChain(uint32_t lastClusterOfChain){
	uint32_t freeCluster=fat_getNextFreeCluster(0);
	assert(fat_getNextCluster(lastClusterOfChain)==FAT_LAST_CLUSTER);
	fat_fat_setValue(lastClusterOfChain,freeCluster);
	fat_fat_setValue(freeCluster,FAT_LAST_CLUSTER);
	return 0;
}
int fat_removeLastClusterFromFile(t_fat_file_entry * file){
	uint32_t lastCluster,theOneBefore;
	lastCluster = fat_getFileLastCluster(file);
	theOneBefore = fat_getClusterPointingTo(lastCluster,fat_getEntryFirstCluster(&file->dataEntry));
	fat_fat_setValue(lastCluster,FAT_FREE_CLUSTER);
	fat_fat_setValue(theOneBefore,FAT_LAST_CLUSTER);
	return 0;
}

int fat_addClusterToFile(t_fat_file_entry * file){
	uint32_t lastCluster = fat_getFileLastCluster(file);
	fat_addFreeClusterToChain(lastCluster);
	return 0;
}

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

void fat_initialize(){
	disk_initialize();
	bootSector = fat_readBootSector();
}


int main(){
	fat_initialize();
	t_fat_file_list * dir, *p;
	char fileName[15];
	//char  content[16385];
	/*if (fat_getFileFromPath("/UNDIR/DOSDIR",&fileEntry)){
		dir = fat_getDirectoryListing(&fileEntry);
	}else{
		dir = NULL;
	}*/
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
			printf("Nombre: %s\n,clusterCount:%d\n",fileName,fat_getClusterCount(&p->fileEntry.dataEntry));
			fat_removeLastClusterFromFile(&p->fileEntry);
			printf("Nombre: %s\n,clusterCount:%d\n",fileName,fat_getClusterCount(&p->fileEntry.dataEntry));
			break;
		}
		p=p->next;

	}
	fat_destroyFileList(dir);

	return 0;

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
		fat_addressing_readCluster(clusterN,&cluster,bootSector);
		for(i =0; i<sizeof(t_cluster);i+=sizeof(t_fat_file_data_entry)){
				memcpy(&tempDataEntry,cluster+i,sizeof(t_fat_file_data_entry));
				if (tempDataEntry.name[0]==0){
					return directory;
				}
				if (!inEntry){
					p=(t_fat_file_list * )malloc(sizeof(t_fat_file_list));
					memset(&p->fileEntry,0,sizeof(t_fat_file_entry));
					inEntry=1;
				}
				if (tempDataEntry.attributes==0x0F) {
					memcpy(&(p->fileEntry.longNameEntry),&tempDataEntry,sizeof(t_fat_long_name_entry));
					p->fileEntry.hasLongNameEntry=1;
				}else{
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
	t_fat_file_entry * temp;
	uint32_t clusterN;
	char *running, *start;
	char *token;
	start = running = strdup(path);

	token = strsep (&running, "/");
	while((token = strsep (&running, "/"))){
		temp = fat_findInDir(dir,token);

		if (temp==NULL){
			fat_destroyFileList(dir);
			free(start);
			return 0;
		}
		memcpy(&fileEntry,temp,sizeof(t_fat_file_entry));
		fat_destroyFileList(dir);
		clusterN=fat_getEntryFirstCluster(&fileEntry.dataEntry) +fat_getRootDirectoryFirstCluster()-2;
		//printf("%s:%d",token,clusterN);
		dir = fat_getFileListFromDirectoryCluster(clusterN);
	}
	memcpy(rtn,&fileEntry,sizeof(t_fat_file_entry));
	free(start);
	fat_destroyFileList(dir);
	return 1;
}
t_fat_file_list * fat_getDirectoryListing(t_fat_file_entry * fileEntry){
	uint32_t clusterN;
	if (fileEntry==NULL)
		return NULL;
	clusterN=fat_getEntryFirstCluster(&fileEntry->dataEntry) +fat_getRootDirectoryFirstCluster()-2;
	return fat_getFileListFromDirectoryCluster(clusterN);
}


t_fat_file_data_entry * fat_getNextEntry(const t_fat_file_list * dir){
	return 0;
}

t_stat fat_statFile(t_fat_file_entry * file){
	t_stat fileStat;
	if (file->dataEntry.attributes==0x10){
		fileStat.st_mode = S_IFDIR | 0755;
		fileStat.st_nlink = 2;
		fileStat.st_size = 4096; //TODO: Cambiar a Cluster Size
	}else if (file->dataEntry.attributes==0x20){
		fileStat.st_mode = S_IFREG | 0444;
		fileStat.st_nlink = 1;
		fileStat.st_size=file->dataEntry.fileSize;
	}
	return fileStat;
}

int fat_readFileContents(t_fat_file_entry * fileEntry,size_t size, off_t offset, char * buf){
	t_cluster data;
	uint32_t dataCluster,diskCluster, offsetInCluster, sizeToRead;
	uint32_t clusterSize = bootSector.sectorPerCluster * bootSector.bytesPerSector;
	int i;
	uint32_t clustersInRead;
	uint32_t contentPosition=0;
	dataCluster = fat_getEntryFirstCluster(&fileEntry->dataEntry);
	if(offset > fileEntry->dataEntry.fileSize)
		return 0;
	if (offset + size > fileEntry->dataEntry.fileSize){
		sizeToRead = fileEntry->dataEntry.fileSize - offset;
	}else{
		sizeToRead = size;
	}
	clustersInRead = ceil(((float)offset+sizeToRead)/4096)-((float)offset/4096);
// Me ubico en el primer cluster de la lectura
	for (i=0;i< offset/clusterSize;i++){
		dataCluster=fat_getNextCluster(dataCluster);
		if (dataCluster==FAT_LASTCLUSTER) return 0;
	}
	for (i=clustersInRead;i>0;i--){
		diskCluster=dataCluster +fat_getRootDirectoryFirstCluster(bootSector)-2;
		fat_addressing_readCluster(diskCluster,&data,bootSector);
		offsetInCluster = offset % clusterSize;
		memcpy(&(buf[contentPosition]),&(data[offsetInCluster]),4096-offsetInCluster);
		contentPosition+=4096-offsetInCluster;
		offset=offset+contentPosition;
		dataCluster=fat_getNextCluster(dataCluster);
	}
	return sizeToRead;
}

void fat_getName (t_fat_file_entry * fileEntry, char * buff){
	// Primera implementacion, convertir de ABCD____TXT a ABCD.TXT
	// TODO: Implementar usando nombres largos.
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
		space=strchr(name,' ');
		*space='\0';
		space=strchr(ext,' ');
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
	assert(fat_getNextCluster(lastClusterOfChain)==FAT_LASTCLUSTER);
	fat_fat_setValue(lastClusterOfChain,freeCluster);
	fat_fat_setValue(freeCluster,FAT_LASTCLUSTER);
	return 0;
}
int fat_removeLastClusterFromFile(t_fat_file_entry * file){
	uint32_t lastCluster,theOneBefore;
	lastCluster = fat_getFileLastCluster(file);
	theOneBefore = fat_getClusterPointingTo(lastCluster,fat_getEntryFirstCluster(&file->dataEntry));
	fat_fat_setValue(lastCluster,FAT_FREECLUSTER);
	fat_fat_setValue(theOneBefore,FAT_LASTCLUSTER);
	return 0;
}

int fat_addClusterToFile(t_fat_file_entry * file){
	uint32_t lastCluster = fat_getFileLastCluster(file);
	fat_addFreeClusterToChain(lastCluster);
	return 0;
}

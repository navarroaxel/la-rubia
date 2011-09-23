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


t_fat_bootsector bootSector;

void fat_initialize(){
	disk_initialize();
	bootSector = fat_readBootSector();
}


int main2(){
	t_fat_file_list * dir, *p;
	char fileName[12]="";
	fat_initialize();

	printf("FAT start: %d\n",bootSector.sectorPerFAT32);
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
			//printf("Nombre: %.*s\n",11,p->fileEntry.dataEntry.name); //http://stackoverflow.com/questions/3767284/using-printf-with-a-non-null-terminated-string
			//fat_printFileContent(p->content);
			printf("Cantidad de Clusters: %s\n",fileName);
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

t_fat_file_list * fat_getFileListFromDirectoryCluster(t_cluster cluster){
	t_fat_file_entry tempFileEntry;
	t_fat_file_list * directory = NULL;
	t_fat_file_list * p;
	t_fat_file_list * pAnt=NULL;
	int i;
	for(i =0; i<sizeof(t_cluster);i+=sizeof(t_fat_file_entry)){
			memcpy(&tempFileEntry,cluster+i,sizeof(t_fat_file_entry));
			if (tempFileEntry.longNameEntry.sequenceN==0){
				return directory;
			}
			p=(t_fat_file_list * )malloc(sizeof(t_fat_file_list));
			memcpy(&(p->fileEntry),&tempFileEntry,sizeof(t_fat_file_entry));
			/*if(p->dataEntry.attributes==0x0F){
				p->isLongName=1;
			}else{
				p->isLongName=0;
			}*/
			if(pAnt!=NULL){
				pAnt->next=p;
			}else{
				directory=p;
			}
			p->next=NULL;
			pAnt=p;
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
	t_cluster rootDirectory;
	fat_addressing_readCluster(fat_getRootDirectoryFirstCluster(),&rootDirectory,bootSector);
	return fat_getFileListFromDirectoryCluster(rootDirectory);
}

int fat_getFileFromPath(const char * path,t_fat_file_entry * rtn){
	t_fat_file_list * dir = fat_getRootDirectory();
	t_fat_file_entry fileEntry;
	t_fat_file_entry * temp;
	uint32_t clusterN;
	t_cluster cluster;
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
		fat_addressing_readCluster(clusterN,&cluster,bootSector);
		dir = fat_getFileListFromDirectoryCluster(cluster);
	}
	memcpy(rtn,&fileEntry,sizeof(t_fat_file_entry));
	free(start);
	fat_destroyFileList(dir);
	return 1;


}
t_fat_file_list * fat_getDirectoryListing(t_fat_file_entry * fileEntry){
	uint32_t clusterN;
	t_cluster cluster;
	if (fileEntry==NULL)
		return NULL;
	clusterN=fat_getEntryFirstCluster(&fileEntry->dataEntry) +fat_getRootDirectoryFirstCluster()-2;
	fat_addressing_readCluster(clusterN,&cluster,bootSector);
	return fat_getFileListFromDirectoryCluster(cluster);
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
	uint32_t firstCluster, offsetInCluster, sizeToRead;
	if(offset > fileEntry->dataEntry.fileSize)
		return 0;
	if (offset + size > fileEntry->dataEntry.fileSize){
		sizeToRead = fileEntry->dataEntry.fileSize - offset;
	}else{
		sizeToRead = size;
	}

	firstCluster = fat_getEntryFirstCluster(&fileEntry->dataEntry)+fat_getRootDirectoryFirstCluster(bootSector)-2;
	fat_addressing_readCluster(firstCluster,&data,bootSector);
	offsetInCluster = offset & (bootSector.sectorPerCluster * bootSector.sectorPerCluster);
	memcpy(buf,data+offsetInCluster,sizeToRead);
	return sizeToRead;
}

void fat_getName (t_fat_file_entry * fileEntry, char * buff){
	// Primera implementacion, convertir de ABCD____TXT a ABCD.TXT
	// TODO: Implementar usando nombres largos.
	int i,j;
	char name[9], extention[4];
	strncpy(name,fileEntry->dataEntry.name,8);
	name[8]='\0';
	strncpy(extention,fileEntry->dataEntry.extension,3);
	extention[3]='\0';
	i=7;
	while ((i > 0) && (name[i]==' ')) name[i--]='\0';
	i=2;
	while ((i> 0) &&  (extention[i]==' ')) extention[i--]='\0';

	i=0;
	while (name[i]){
		buff[i]=name[i];
		i++;
	}
	buff[i++]='.';
	j=0;
	while(extention[j]){
		buff[i++]=extention[j++];
	}
	buff[i]='\0';
	return;
}

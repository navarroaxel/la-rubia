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

typedef struct stat t_stat;
t_fat32_bootsector bootSector;

void fat_initialize(){
	disk_initialize();
	bootSector = fat_readBootSector();
}


int main2(){
	t_cluster rootDirectory;
	t_fat_file_list * dir, *p;

	fat_initialize();

	printf("FAT start: %d\n",bootSector.sectorPerFAT32);
	fat_addressing_readCluster(fat_getRootDirectoryFirstCluster(bootSector),&rootDirectory,bootSector);
	dir=fat_getFileListFromDirectoryCluster(rootDirectory);
	p=dir;
	while (p!=NULL){
		if (p->isLongName){
			t_fat32_long_name_entry * longNameEntry = (t_fat32_long_name_entry *) &(p->content);
			//printf("Nombre Largo! %.*s%.*s%.*s",10,longNameEntry->nameStart,12,longNameEntry->nameMiddle,4,longNameEntry->nameEnd);


		}else{
			switch (p->content.name[0]) {
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
				printf("Nombre: %.*s\n",11,p->content.name); //http://stackoverflow.com/questions/3767284/using-printf-with-a-non-null-terminated-string
				//fat_printFileContent(p->content);
				printf("Cantidad de Clusters: %d\n",fat_getClusterCount(&(p->content)));
				break;
			}
		}
		p=p->next;

	}
	fat_destroyFileList(dir);
	return 0;
}

t_fat32_bootsector fat_readBootSector(){
	t_fat32_bootsector bs;
	t_sector cluster;
	if (disk_readSector(0,&cluster)){
		memcpy(&bs,&cluster,sizeof(t_fat32_bootsector));
	}
	return bs;
}

void fat_printFileContent(t_fat32_file_entry fileEntry){
	t_cluster data;
	uint32_t firstCluster = fat_getEntryFirstCluster(fileEntry)+fat_getRootDirectoryFirstCluster(bootSector)-2;
	fat_addressing_readCluster(firstCluster,&data,bootSector);
	printf("Contenido: %s",data);
}

t_fat_file_list * fat_getFileListFromDirectoryCluster(t_cluster cluster){
	t_fat32_file_entry tempFileEntry;
	t_fat_file_list * directory = NULL;
	t_fat_file_list * p;
	t_fat_file_list * pAnt=NULL;
	int i;
	for(i =0; i<sizeof(t_cluster);i+=sizeof(t_fat32_file_entry)){
			memcpy(&tempFileEntry,cluster+i,sizeof(t_fat32_file_entry));
			if (tempFileEntry.name[0]==0x00){
				return directory;
			}
			p=(t_fat_file_list * )malloc(sizeof(t_fat_file_list));
			memcpy(&(p->content),&tempFileEntry,sizeof(t_fat32_file_entry));
			if(p->content.attributes==0x0F){
				p->isLongName=1;
			}else{
				p->isLongName=0;
			}
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

t_fat_file_list * fat_getDirectoryListing(char * path){
	return 0;
}

t_fat32_file_entry * fat_getNextEntry(const t_fat_file_list * dir){

	return 0;
}

t_stat fat_statFile(t_fat32_file_entry * file){
	return 0;
}

/*void loadFAT(){
	t_cluster cluster;
	uint8_t localFAT[524288];
	uint32_t fatStart = fat_getFATFirstCluster(bootSector);
	for(uint32_t i = 0 ; i < FatClustersSize;i++){
		fat_addressing_readCluster(i + fatStart,&cluster,bootSector);
		printf("%d %d\n ",i,FAT+i*128);
		if (i==127){
			puts("");
		}
		memcpy(localFAT+i*4096,&cluster,4096);//Desplaza por tipo de dato, en un sector
	}
	memcpy(FAT,localFAT,524288);
}*/

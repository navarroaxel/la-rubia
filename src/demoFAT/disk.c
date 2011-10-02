/*
 * disk.c
 *
 *  Created on: Sep 2, 2011
 *      Author: nico
 */


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include "fatTypes.h"
#include "disk.h"

/**
 * Read Sector into buffer;
 */
struct disk_config{
	char file[255];
	uint32_t sectorSize;
	//y el resto
} disk_config_data;

static int disk_is_initialized=0;

int disk_initialize(){
	strcpy(disk_config_data.file,"/home/nico/fat32.disk");
	disk_config_data.sectorSize=512;
	disk_is_initialized=1;
	return 0;
}

int disk_isInitialized(){
	return disk_is_initialized;
}

int disk_readSector(uint32_t sector, t_sector *  buf){
	FILE *f;
	int n;
	int rseek;
	uint32_t byteStart=sector*disk_config_data.sectorSize;
	if((f=fopen(disk_config_data.file,"r"))){
		if ((rseek=fseek(f,byteStart,SEEK_SET))==0){
			if ((n=fread(buf,disk_config_data.sectorSize,1,f))){
				fclose(f);
				return n;
			}else{
				perror("No pude Leer el archivo");
			}
		}else{
			perror("No pude hacer el seek");
		}
	}else{
		perror("No pude abrir el archivo");
	}
	fclose(f);
	return 0;
}

int disk_writeSector(uint32_t sector,t_sector * buf){
	FILE *f;
	int n;
	int rseek;
	uint32_t byteStart=sector*disk_config_data.sectorSize;
	if((f=fopen(disk_config_data.file,"r+"))){
		if ((rseek=fseek(f,byteStart,SEEK_SET))==0){
			if ((n=fwrite(buf,disk_config_data.sectorSize,1,f))){
				fclose(f);
				return n;
			}else{
				perror("No pude Leer el archivo");
			}
		}else{
			perror("No pude hacer el seek");
		}
	}else{
		perror("No pude abrir el archivo");
	}
	fclose(f);
	return 0;
}

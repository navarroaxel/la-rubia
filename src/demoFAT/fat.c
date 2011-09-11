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
#include "fat.h"
#include "fatHelpers.h"
#include "fatAddressing.h"
#include "disk.h"


int main(){
	t_fat32_bootsector bootSector;
	disk_initialize();
	bootSector = fat_readBootSector();

	printf("data start: %d",fat_getRootDirectoryFirstCluster(bootSector));
	return 0;
}

t_fat32_bootsector fat_readBootSector(){
	t_fat32_bootsector bs;
	cluster_t cluster;
	if (disk_readSector(0,cluster)){
		memcpy(&bs,&cluster,sizeof(t_fat32_bootsector));
	}
	return bs;
}


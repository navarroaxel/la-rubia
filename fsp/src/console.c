#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "console.h"
#include "fat.h"
#include "fatHelpers.h"

extern t_fat_bootsector bootSector;

void * console(void * t) {
	t_commands *cmd =commands_create(' ','\n', ' ');

	commands_add(cmd, "fsinfo", fsinfo);
	commands_add(cmd, "finfo", finfo);

	char input[100];
	while (true) {
		printf("> ");
		fflush(stdin);
		fgets(input, sizeof(input), stdin);
		commands_parser(cmd, input);
	}
	return NULL;
}

void fsinfo(void *context, t_array *args) {
	printf("Clusters Usados: %u\n",fat_getUsedClusterCount());
	printf("Clusters Libres: %u\n",fat_getFreeClusterCount());
	printf("Tamaño Sector: %u\n",bootSector.bytesPerSector);
	printf("Tamaño FAT: %u\n", (bootSector.totalSectors32/bootSector.sectorPerCluster)*FAT_CLUSTER_SIZE / 1024);
	return;
}

void finfo(void *context, t_array *args) {
	if (args != NULL && array_size(args) == 1) {
		char * path = (char *) array_get(args,0);
		t_fat_file_entry fileEntry;
		uint32_t clusterN, i;
		if(!fat_getFileFromPath(path,&fileEntry)) return;
		clusterN = fat_getEntryFirstCluster(&fileEntry.dataEntry);
		if (clusterN==0)
			return;

		for (i=0;i<20;i++){
			if(clusterN ==FAT_LAST_CLUSTER) return;
			printf("cluster %u: %u\n",i,clusterN);
			clusterN=fat_getNextCluster(clusterN);
		}

	}
}

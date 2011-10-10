/*
 * fat.h
 *
 *  Created on: Sep 5, 2011
 *      Author: nico
 */

#ifndef FAT_H_
#define FAT_H_
#include "fatTypes.h"

t_fat_bootsector fat_readBootSector();
typedef struct stat t_stat;


t_fat_file_list * fat_getFileListFromDirectoryCluster(uint32_t clusterN);
void fat_destroyFileList(t_fat_file_list * fileList);
void fat_initialize();
t_fat_file_list * fat_getDirectoryListing(t_fat_file_entry * fileEntry);
int fat_getFileFromPath(const char * path,t_fat_file_entry * rtn);
t_stat fat_statFile(t_fat_file_entry * file);
int fat_readFileContents(t_fat_file_entry * fileEntry,size_t size, off_t offset, char * buf);
void fat_getName (t_fat_file_entry * fileEntry, char * buff);
t_fat_file_list * fat_getRootDirectory();
int fat_addFreeClusterToChain(uint32_t lastClusterOfChain);
int fat_addClusterToFile(t_fat_file_entry * file);
int fat_removeLastClusterFromFile(t_fat_file_entry * file);
void freeArrayofPointersToStrings(char ** caca,size_t arraySize);

typedef struct {
	char diskIp[16];
	uint16_t diskPort;
	uint16_t bindPort;
	uint32_t cacheSizeInClusters;
} t_fat_config;
#endif /* FAT_H_ */

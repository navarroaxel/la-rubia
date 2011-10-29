/*
 * fat.h
 *
 *  Created on: Sep 5, 2011
 *      Author: nico
 */

#ifndef FAT_H_
#define FAT_H_
#include "fatTypes.h"
#include <stdint.h>
#include "common/utils/config.h"
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
void fat_loadFAT();
void fat_initialize();
int fat_truncate(const char * path,off_t size);
void fat_getRootDirectoryEntry(t_fat_file_entry * rootDirectoryEntry);
int fat_setFileEntry(const char *path, t_fat_file_entry * fileEntry);
int fat_move(const char * from,const char * to);
int fat_addEntry(const char * directoryPath, t_fat_file_entry fileEntry);
int fat_write(const char *path, const char *buf, size_t size, off_t offset);
int fat_mkdir(const char * path);
void fat_cleanup(void);
#endif /* FAT_H_ */

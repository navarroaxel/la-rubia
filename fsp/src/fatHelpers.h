/*
 * fatHelpers.h
 *
 *  Created on: Sep 5, 2011
 *      Author: nico
 */

#ifndef FATHELPERS_H_
#define FATHELPERS_H_

uint32_t fat_getEntryFirstCluster(t_fat_file_data_entry * fileEntry );
uint32_t fat_getRootDirectoryFirstCluster();
uint32_t fat_getFATFirstCluster();
uint32_t fat_getNextCluster(uint32_t currentCluster);
uint32_t fat_getClusterCount(t_fat_file_data_entry * file);
t_fat_file_entry * fat_findInDir(const t_fat_file_list * dir,char * name);
uint32_t fat_getNextFreeCluster(uint32_t start);
uint32_t fat_getFileLastCluster(t_fat_file_entry * file);
int fat_fat_setValue(uint32_t clusterN,uint32_t next);
uint32_t fat_getClusterPointingTo(uint32_t clusterToFind, uint32_t clusterofChain);
uint32_t fat_getFATAddressOfEntry(uint32_t clusterN);
uint32_t fat_dataClusterToDiskCluster(uint32_t dataCluster);
void splitPathName(const char * path,char * dir, char * file);
void splitNameExtension(const char * fullName,char * name, char * extension);
int fat_renameFile(const char * newName, t_fat_file_entry * destinationDir, t_fat_file_entry * fileEntry);
int fat_shortNameExists(char * nameWithoutExtension,t_fat_file_entry * dir);
int fat_generateShortName(const char * newName, t_fat_file_entry * dir, char * shortName);
int needsLongName(const char * name);
void stringToUpper(char * string);
uint8_t lfn_checksum(const unsigned char *pFcbName);
uint32_t fat_getFreeClusterCount();
uint32_t fat_getUsedClusterCount();

#endif /* FATHELPERS_H_ */

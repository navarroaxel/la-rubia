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
uint32_t getFATAddressOfEntry(uint32_t clusterN);
uint32_t fat_dataClusterToDiskCluster(uint32_t dataCluster);

#endif /* FATHELPERS_H_ */

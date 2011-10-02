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


#endif /* FATHELPERS_H_ */

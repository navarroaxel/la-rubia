/*
 * fatHelpers.h
 *
 *  Created on: Sep 5, 2011
 *      Author: nico
 */

#ifndef FATHELPERS_H_
#define FATHELPERS_H_

uint32_t fat_getEntryFirstCluster(t_fat32_file_entry fileEntry );
uint32_t fat_getRootDirectoryFirstCluster(t_fat32_bootsector bs);
uint32_t fat_getFATFirstCluster(t_fat32_bootsector bs);
uint32_t fat_getNextCluster(uint32_t currentCluster);


#endif /* FATHELPERS_H_ */

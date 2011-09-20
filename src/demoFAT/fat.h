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


void fat_printFileContent(t_fat_file_data_entry fileEntry);
t_fat_file_list * fat_getFileListFromDirectoryCluster(t_cluster cluster);
void fat_destroyFileList(t_fat_file_list * fileList);
void fat_initialize();



#endif /* FAT_H_ */

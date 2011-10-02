/*
 * fatAddressing.h
 *
 *  Created on: Sep 7, 2011
 *      Author: nico
 */

#ifndef FATADDRESSING_H_
#define FATADDRESSING_H_

#define FAT_LASTCLUSTER 0x0FFFFFFF
int fat_addressing_readCluster(uint32_t clusterNumber, t_cluster * buffer,t_fat_bootsector bs);
int fat_addressing_writeCluster(uint32_t clusterNumber, t_cluster * buffer,t_fat_bootsector bs);
#endif /* FATADDRESSING_H_ */

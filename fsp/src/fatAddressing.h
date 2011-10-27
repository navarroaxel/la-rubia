/*
 * fatAddressing.h
 *
 *  Created on: Sep 7, 2011
 *      Author: nico
 */

#ifndef FATADDRESSING_H_
#define FATADDRESSING_H_

int fat_addressing_readCluster(uint32_t clusterNumber, t_cluster buffer);
int fat_addressing_writeCluster(uint32_t clusterNumber, t_cluster buffer);
int fat_addressing_readBlock(uint32_t blockNumber, t_block buffer);
int fat_addressing_writeBlock(uint32_t blockNumber, t_block buffer);
uint32_t fat_getRootDirectoryFirstCluster();
#endif /* FATADDRESSING_H_ */

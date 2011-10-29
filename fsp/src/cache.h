/*
 * cache.h
 *
 *  Created on: Oct 2, 2011
 *      Author: nico
 */

#ifndef CACHE_H_
#define CACHE_H_

#include "fatTypes.h"
int cache_read(uint32_t clusterNumber, t_cluster buffer);
int cache_write(uint32_t clusterNumber, t_cluster buffer);
void cache_initialize();
int cache_isInitialized();

typedef struct {
	uint32_t blockN;
	t_cluster data;
} t_cache_entry;



#endif /* CACHE_H_ */

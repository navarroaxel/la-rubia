/*
 * cache.c
 *
 *  Created on: Oct 2, 2011
 *      Author: nico
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <stdint.h>
#include "cache.h"

int cache_initialized = 0;
t_cache_entry cache[1024];

int cache_read(uint32_t clusterNumber, t_cluster * buffer){
	if (cache[clusterNumber % 1024].clusterN==clusterNumber){
		memcpy(buffer,cache[clusterNumber % 1024].data,4096);
		return 1;
	}
	return 0;
};

int cache_write(uint32_t clusterNumber, t_cluster * buffer){
	cache[clusterNumber % 1024].clusterN=clusterNumber;
	memcpy(&(cache[clusterNumber % 1024].data),buffer,4096);
	return 1;
}

void cache_initialize() {
	int i;
	for (i=0;i<1024;i++){
		cache[i].clusterN=0;
		memset(cache[i].data,0,1024);
	}
	cache_initialized = 1;
}

int cache_isInitialized(){
	return cache_initialized;
}

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
#include <semaphore.h>
#define CACHESIZE 1024
int cache_initialized = 0;
t_cache_entry cache[CACHESIZE];
sem_t poolResources;

int cache_read(uint32_t clusterNumber, t_cluster buffer){
    sem_wait(&poolResources);
	if (cache[clusterNumber % CACHESIZE].blockN==clusterNumber){
		memcpy(buffer,&cache[clusterNumber % CACHESIZE].data,FAT_CLUSTER_SIZE);
		sem_post(&poolResources);
		return 1;
	}
	sem_post(&poolResources);
	return 0;
};

int cache_write(uint32_t clusterNumber, t_cluster buffer){
	sem_wait(&poolResources);
	cache[clusterNumber % CACHESIZE].blockN=clusterNumber;
	memcpy(&(cache[clusterNumber % CACHESIZE].data),buffer,FAT_CLUSTER_SIZE);
	sem_post(&poolResources);
	return 1;
}

void cache_initialize() {
	int i;
	for (i=0;i<CACHESIZE;i++){
		cache[i].blockN=0xFFFFFFFF;
		memset(cache[i].data,0,FAT_CLUSTER_SIZE);
	}
	sem_init(&poolResources, 0, 1);
	cache_initialized = 1;
}

int cache_isInitialized(){
	return cache_initialized;
}

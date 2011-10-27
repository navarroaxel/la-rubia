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
#define CACHESIZE 4096
int cache_initialized = 0;
t_cache_entry cache[CACHESIZE];
sem_t semaforo;

int cache_read(uint32_t blockNumber, t_block buffer){
    sem_wait(&semaforo);
	if (cache[blockNumber % CACHESIZE].blockN==blockNumber){
		memcpy(buffer,&cache[blockNumber % CACHESIZE].data,FAT_BLOCK_SIZE);
		sem_post(&semaforo);
		return 1;
	}
	sem_post(&semaforo);
	return 0;
};

int cache_write(uint32_t blockNumber, t_block buffer){
	sem_wait(&semaforo);
	cache[blockNumber % CACHESIZE].blockN=blockNumber;
	memcpy(&(cache[blockNumber % CACHESIZE].data),buffer,FAT_BLOCK_SIZE);
	sem_post(&semaforo);
	return 1;
}

void cache_initialize() {
	int i;
	for (i=0;i<CACHESIZE;i++){
		cache[i].blockN=0xFFFFFFFF;
		memset(cache[i].data,1,FAT_BLOCK_SIZE);
	}
	sem_init(&semaforo, 0, 1);
	cache_initialized = 1;
}

int cache_isInitialized(){
	return cache_initialized;
}

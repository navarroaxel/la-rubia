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

int cache_initialized = 0;
t_cache_entry cache[1024];//TODO: Tamaño deacuerdo a archivo de config
sem_t semaforo;

int cache_read(uint32_t clusterNumber, t_cluster * buffer){//TODO: Tamaño deacuerdo a archivo de config
    sem_wait(&semaforo);
	if (cache[clusterNumber % 1024].clusterN==clusterNumber){
		memcpy(buffer,cache[clusterNumber % 1024].data,FAT_CLUSTER_SIZE);
		sem_post(&semaforo);
		return 1;
	}
	sem_post(&semaforo);
	return 0;
};

int cache_write(uint32_t clusterNumber, t_cluster * buffer){
	sem_wait(&semaforo);
	cache[clusterNumber % 1024].clusterN=clusterNumber;
	memcpy(&(cache[clusterNumber % 1024].data),buffer,FAT_CLUSTER_SIZE);
	sem_post(&semaforo);
	return 1;
}

void cache_initialize() {//TODO: Tamaño deacuerdo a archivo de config
	int i;
	for (i=0;i<1024;i++){
		cache[i].clusterN=0;
		memset(cache[i].data,0,1024);
	}
	sem_init(&semaforo, 0, 1);
	cache_initialized = 1;
}

int cache_isInitialized(){
	return cache_initialized;
}

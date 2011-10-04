/*
 ============================================================================
 Name        : raid.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "common/nipc.h"
#include <assert.h>

int main(void) {
	t_disk_readSectorRq *rq = malloc(sizeof(t_disk_readSectorRq));
	rq->offset = 4;

	t_nipc *nipc = nipc_create(NIPC_READSECTOR_RQ);
	t_disk_readSectorRq *rq2;
	t_nipc *nipc2;
	nipc_setdata(nipc,rq, sizeof(t_disk_readSectorRq));

	t_socket_buffer *buffer = (t_socket_buffer *)nipc_serializer(nipc);
	nipc2 = (t_nipc *)nipc_deserializer(buffer);
	rq2 = nipc2->payload;

	assert(nipc->type == nipc2->type);
	assert(nipc->length == nipc2->length);
	assert(rq2->offset == rq->offset);

	return EXIT_SUCCESS;
}

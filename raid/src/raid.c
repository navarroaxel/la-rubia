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
	t_socket_client *client = sockets_createClient("127.0.0.1", 5200);

	sockets_connect(client, "127.0.0.1", 5200);
	t_disk_readSectorRq *rq = malloc(sizeof(t_disk_readSectorRq));
	rq->offset = 0;

	t_nipc *nipc = nipc_create(NIPC_READSECTOR_RQ);
	t_disk_readSectorRs *rs;
	t_nipc *nipc2;
	nipc_setdata(nipc, rq, sizeof(t_disk_readSectorRq));

	t_socket_buffer *buffer = (t_socket_buffer *)nipc_serializer(nipc);
	sockets_send(client, buffer->data, buffer->size);

	buffer = sockets_recv(client);
	nipc2 = nipc_deserializer(buffer);
	rs = nipc2->payload;

	printf("%.*s\n", DISK_SECTOR_SIZE, rs->data);
	printf("%1024X\n", rs->data);
	printf("%i\n", rs->offset);
	printf("%i\n", rs->result);

	return EXIT_SUCCESS;
}

void listener(t_blist *waiting) {
	//TODO: Get IP & port from config.
	t_socket_server *server = sockets_createServer("192.168.0.146", 5678);

	sockets_listen(server);

	t_list *servers = collection_list_create();
	collection_list_add(servers, server);

	int recvClient(t_socket_client * client) {
		t_socket_buffer *buffer = sockets_recv(client);
		if (buffer == NULL)
			return 0;

		t_nipc *nipc = nipc_deserializer(buffer);
		if (nipc->type == 0)
		{
			if (handshake(client) == 0)
			return 0;
		}

		t_disk_operation *op = getdiskoperation(nipc, client);
		if (op == NULL)
			return 0;

		nipc_destroy(nipc);
		enqueueOperation(waiting, op);

		sockets_bufferDestroy(buffer);
		return client->socket->desc;
	}

	t_list *clients = collection_list_create();

	while (true) {
		sockets_select(servers, clients, 0, NULL, &recvClient);
	}
}

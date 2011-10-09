#include "listener.h"

void listener(t_blist *waiting) {
	/*t_socket_server *sckt = sockets_createServerUnix(SOCKET_UNIX_PATH);

	sockets_listen(sckt);
	printf("escuchando");
	t_socket_client *a = sockets_acceptUnix(sckt);
	t_socket_buffer *b = sockets_recv(a);
	printf("%s\n", b->data);
	printf("se acabo el socket unix");*/

	//TODO: Get IP & port from config.
	t_socket_server *server = sockets_createServer("127.0.0.1", 5800);

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
			if (handshake(client, nipc) == 0)
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

int handshake(t_socket_client *client, t_nipc *nipc2) {
	//TODO: Revisar si estoy en modo CONNECT.
	t_nipc *nipc = nipc_create(NIPC_HANDSHAKE);
	nipc_setdata(nipc, NULL, 0);
	//TODO: la serializacion y el envio del buffer se repite, pasar a una funcion.
	t_socket_buffer *buffer = nipc_serializer(nipc);
	sockets_sendBuffer(client, buffer);
	sockets_bufferDestroy(buffer);
	nipc_destroy(nipc);
	return 0;
}

t_disk_operation *getdiskoperation(t_nipc *nipc, t_socket_client *client) {
	t_disk_operation *operation = malloc(sizeof(t_disk_operation));
	switch (nipc->type) {
	case NIPC_READSECTOR_RQ:
		operation->read = true;
		operation->offset = ((t_disk_readSectorRq *) nipc->payload)->offset;
		break;
	case NIPC_WRITESECTOR_RQ:
		operation->read = false;
		t_disk_writeSectorRq *rq = (t_disk_writeSectorRq *) nipc->payload;
		operation->offset = rq->offset;
		memcpy(&operation->data, rq->data, DISK_SECTOR_SIZE);
		break;
	default:
		return NULL;
	}
	operation->client = client;
	return operation;
}

void enqueueOperation(t_blist *waiting, t_disk_operation *op) {
	//encola la operacion en base al algoritmo de R&W utilizado.

	int enqueueDiskOperation(void *data) {
		return op->offset >= ((t_disk_operation *) data)->offset;
	}
	collection_blist_put(waiting, op, &enqueueDiskOperation);
}

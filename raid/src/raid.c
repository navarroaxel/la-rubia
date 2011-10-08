#include "raid.h"

int main(void) {

	return EXIT_SUCCESS;
}

void listener(t_blist *waiting) {
	//TODO: Get IP & port from config.
	t_socket_server *server = sockets_createServer("127.0.0.1", 5100);

	sockets_listen(server);

	t_list *servers = collection_list_create();
	collection_list_add(servers, server);

	int recvClient(t_socket_client * client) {
		t_socket_buffer *buffer = sockets_recv(client);
		if (buffer == NULL
			)
			return 0;

		t_nipc *nipc = nipc_deserializer(buffer);
		if (nipc->type == 0)
			return handshake(client);

		enqueueoperation(nipc, client);

		nipc_destroy(nipc);
		sockets_bufferDestroy(buffer);

		return client->socket->desc;
	}

	t_list *clients = collection_list_create();

	while (true) {
		sockets_select(servers, clients, 0, NULL, &recvClient);
	}
}

int handshake(t_socket_client *client, t_nipc *rq) {
	if (rq->length > 0) {
		char diskid[rq->length];
		memcpy(diskid, rq->payload, rq->length);
		registerdisk(diskid, client);
	} else if (collection_list_size(disks) == 0) {
		t_nipc *nipc = nipc_create(NIPC_HANDSHAKE);
		nipc_setdata(nipc, "There are no disks ready.",
				strlen("There are no disks ready.") + 1);
		nipc_send(nipc, client);
		nipc_destroy(nipc);
		return 0;
	}

	t_nipc *nipc = nipc_create(NIPC_HANDSHAKE);
	nipc_setdata(nipc, NULL, 0);
	nipc_send(nipc, client);
	nipc_destroy(nipc);
	return client->socket->desc;
}

void registerdisk(char *id, t_socket_client *client) {
	struct t_disk *dsk = malloc(sizeof(struct t_disk));

	//TODO: validate duplicated id.

	memcpy(&dsk->id, id, strlen(id) + 1);
	dsk->waiting = collection_blist_create(50);
	dsk->client = client;

	pthread_t *thread;
	pthread_create(thread, NULL, &disk, dsk->waiting);

	dsk->thread = thread;
	collection_list_add(disks, dsk);
}

void enqueueoperation(t_nipc *nipc, t_socket_client *client) {
	if (nipc->type == NIPC_READSECTOR_RQ)
	{
		int count = INT_MAX;
		struct t_disk *disk;
		void find_idledisk(void *data) {
			struct t_disk *d = (struct t_disk *) data;
			if (collection_blist_size(d->waiting) < INT_MAX) {
				disk = data;
				count = collection_blist_size(d->waiting);
			}
		}

		collection_list_iterator(disks, find_idledisk);
		t_socket_buffer *buffer = nipc_serializer(nipc);
		sockets_sendBuffer(disk->client, buffer);
		collection_blist_push(disk->waiting, nipc_clone(nipc));
	} else if (nipc->type == NIPC_WRITESECTOR_RQ)
	{
		t_socket_buffer *buffer = nipc_serializer(nipc);
		void sendrequest(void *data) {
			struct t_disk *disk = (struct t_disk *) data;
			sockets_sendBuffer(disk->client, buffer);
			collection_blist_push(disk->waiting, nipc_clone(nipc));
		}
		collection_list_iterator(disks, sendrequest);
	}
}


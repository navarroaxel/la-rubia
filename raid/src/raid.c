#include "raid.h"

int disk_id = 1;

int main(void) {
	t_list *waiting = collection_list_create();

	listener(waiting);

	return EXIT_SUCCESS;
}

void diskconnect(void) {
	t_socket_client *client = sockets_createClient("127.0.0.1", 5500);

	sockets_connect(client, "127.0.0.1", 5800);
	t_disk_readSectorRq *rq = malloc(sizeof(t_disk_readSectorRq));
	rq->offset = 0;

	t_nipc *nipc = nipc_create(NIPC_READSECTOR_RQ);
	t_disk_readSectorRs *rs;
	t_nipc *nipc2;
	nipc_setdata(nipc, rq, sizeof(t_disk_readSectorRq));

	t_socket_buffer *buffer = nipc_serializer(nipc);
	sockets_send(client, buffer->data, buffer->size);

	buffer = sockets_recv(client);
	nipc2 = nipc_deserializer(buffer);
	rs = nipc2->payload;
}

void listener(t_list *waiting) {
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
		if (nipc->type == NIPC_HANDSHAKE
		)
			return handshake(client, nipc);

		enqueueoperation(nipc, client);

		nipc_destroy(nipc);
		sockets_bufferDestroy(buffer);

		return 0;
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

void registerdisk(char *name, t_socket_client *client) {
	struct t_disk *dsk = malloc(sizeof(struct t_disk));

	//TODO: validate duplicated id.

	memcpy(&dsk->name, name, strlen(name) + 1);
	dsk->id = disk_id;
	dsk->waiting = NULL;
	dsk->client = client;

	disk_id <<= 1;

	pthread_t thread;
	pthread_create(&thread, NULL, &disk, dsk);

	dsk->thread = thread;
	collection_list_add(disks, dsk);
}

void enqueueoperation(t_nipc *nipc, t_socket_client *client) {
	t_operation *op = operation_create(nipc);
	if (op->read) {
		int count = INT_MAX;
		struct t_disk *disk;
		void find_idledisk(void *data) {
			t_disk *d = data;
			if (d->pendingreads < count) {
				disk = data;
				count = d->pendingreads;
			}
		}

		collection_list_iterator(disks, find_idledisk);
		disk->pendingreads++;
		op->disk = disk->id;
		nipc_send(nipc, disk->client);
	} else {
		t_socket_buffer *buffer = nipc_serializer(nipc);
		void sendrequest(void *data) {
			struct t_disk *disk = (struct t_disk *) data;
			sockets_sendBuffer(disk->client, buffer);
			op->disk |= disk->id;
		}
		collection_list_iterator(disks, sendrequest);
	}
}

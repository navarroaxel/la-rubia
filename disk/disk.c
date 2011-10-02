#include "disk.h"
#include "stdio.h"

int main_disk(void) {
	t_blist *operations = collection_blist_create(50);
	t_blist *ready = collection_blist_create(50);

	init_head(operations, ready);

	init_dispatcher(ready);

	socketlisten(operations);

	return EXIT_SUCCESS;
}

void socketlisten(t_blist *list) {
	t_socket_server *server = sockets_createServer("127.0.0.1", 5200);

	sockets_listen(server);

	t_list *servers = collection_list_create();
	collection_list_add(servers, server);

	int recvClient(t_socket_client * client) {
		t_socket_buffer *buffer = sockets_recv(client);
		if (buffer == NULL)
			return 0;

		//TODO Translate to disk_operation
		t_disk_operation *op = (t_disk_operation *)malloc(sizeof(t_disk_operation));
		op->client = client;

		enqueueOperation(list, op);

		sockets_bufferDestroy(buffer);
		return client->socket->desc;
	}

	t_list *clients = collection_list_create();

	while (true) {
		sockets_select(servers, clients, 0, NULL, &recvClient);
	}
}

void enqueueOperation(t_blist *list, t_disk_operation *op) {
	//encola la operacion en base al algoritmo de R&W utilizado.

	int enqueueDiskOperation(void *data){
		return op->offset >= ((t_disk_operation *)data)->offset;
	}

	collection_blist_put(list, op, &enqueueDiskOperation);
}

struct queues{
	t_blist *operations;
	t_blist *readyQueue;
};

void init_head(t_blist *operations, t_blist *ready) {
	pthread_attr_t attr;
	pthread_t head_id;

	struct queues *q = (struct queues *)malloc(sizeof(struct queues));
	q->operations = operations;
	q->readyQueue = ready;

	pthread_attr_init (&attr);
	pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
	pthread_create (&head_id, &attr, &head, q);
	pthread_attr_destroy (&attr);
}

void init_dispatcher(t_blist *ready){
	pthread_attr_t attr;
	pthread_t dispatcher_id;

	pthread_attr_init (&attr);
	pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
	pthread_create (&dispatcher_id, &attr, &dispatcher, ready);
	pthread_attr_destroy (&attr);
}

void *dispatcher(void *args) {
	t_blist *ready = (t_blist *)args;
	t_disk_operation *e;
	while (true) {
		e = (t_disk_operation *)collection_blist_pop(ready);
		//TODO: send the result of the operation;
	}
	return NULL;
}

void *head(void *args) {
	struct queues *q = (struct queues *)args;
	t_disk_operation *e;
	while (true) {
		e = (t_disk_operation *)collection_blist_pop(q->operations);
		printf("dequeue \"%i\"\n", e->offset);
		collection_blist_push(q->readyQueue, e);
	}
	return NULL;
}

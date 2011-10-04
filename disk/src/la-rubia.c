#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "common/utils/queue.h"
#include "common/utils/sockets.h"
#include <string.h>

void *produce(void* queue);
void *consume(void* queue);
void start_producer(t_queue *q, char* t, t_socket_client *client);
void start_consumer(t_queue *q);

struct data {
	t_queue *queue;
	char *text;
	t_socket_client *client;
};

int main_rubia(void) {
	t_queue *queue = queue_create(20);
	t_socket_server *server = sockets_createServer("127.0.0.1", 5200);

	start_consumer(queue);

	sockets_listen(server);

	t_list *servers = collection_list_create();
	collection_list_add(servers, server);

	int recvClient(t_socket_client * client) {
		t_socket_buffer *buffer = sockets_recv(client);
		if (buffer == NULL)
			return 0;

		char* text = malloc(strlen(buffer->data) + 1);
		memcpy(text, buffer->data, strlen(buffer->data) + 1);

		start_producer(queue, text, client);

		sockets_bufferDestroy(buffer);
		return client->socket->desc;
	}

	t_list *clients = collection_list_create();

	while (1) {
		sockets_select(servers, clients, 0, NULL, &recvClient);
	}

	return EXIT_SUCCESS;
}

void start_producer(t_queue *q, char* t, t_socket_client *client){
	pthread_attr_t attr;
	pthread_t producer;
	struct data *data;
	data = (struct data *)malloc(sizeof(struct data));
	data->queue = q;
	data->text = t;
	data->client = client;

	pthread_attr_init (&attr);
	pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
	pthread_create (&producer, &attr, &produce, data);
	pthread_attr_destroy (&attr);
}

void start_consumer(t_queue *q) {
	pthread_attr_t attr;
	pthread_t consumer;

	pthread_attr_init (&attr);
	pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
	pthread_create (&consumer, &attr, &consume, q);
	pthread_attr_destroy (&attr);
}

void *produce(void* args){
	struct data *data = (struct data *)args;
	t_queue *q = data->queue;

	queue_push(q, data->text);
	sockets_send(data->client, "Processed!\n", strlen("Processed!\n")+1);

	free(data);
	return NULL;
}

void *consume(void* queue){
	t_queue *q = (t_queue *)queue;
	char *e;
	while (1) {
		e = queue_pop(q);
		sleep(3);
		printf("dequeue \"%s\"\n", e);
		free(e);
	}
	return NULL;
}

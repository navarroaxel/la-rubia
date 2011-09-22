#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "queue.h"
#include "sockets.h"

void *produce(void* queue);
void *consume(void* queue);
void start_producer(t_queue *q, char* t);
void start_consumer(t_queue *q);

struct data {
	t_queue *queue;
	char *text;
};

int main(void) {
	t_queue *queue = queue_create(20);
	t_socket_server *server = sockets_createServer("127.0.0.1", 5200);
	t_socket_buffer *buffer;
	t_socket_client *client;

	start_consumer(queue);

	while (1) {
		sockets_listen(server);

		client = sockets_accept(server);

		buffer = sockets_recv(client);

		char* text;
		memcpy(text, buffer->data, strlen(buffer->data) + 1);

		start_producer(queue, text);

		sockets_bufferDestroy(buffer);

		sockets_destroyClient(client);
	};

	sockets_destroyServer(server);

	return EXIT_SUCCESS;
}

void start_producer(t_queue *q, char* t){
	pthread_attr_t attr;
	pthread_t producer;
	struct data *data;
	data = (struct data *)malloc(sizeof(struct data));
	data->queue = q;
	data->text = t;

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
	printf("enqueue \"%s\"\n", data->text);
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

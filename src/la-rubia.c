#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "queue.h"
#include "sockets.h"

void *produce(void* queue);
void *consume(void* queue);

int main(void) {
	t_socket_server *server = sockets_create("127.0.0.1", 8000);
	t_socket_buffer *buffer;
	t_socket_client *client;

	sockets_listen(server);

	client = sockets_accept(server);

	buffer = sockets_recv(client);

	printf("%s", buffer->data);

	sockets_bufferDestroy(buffer);

	sockets_destroyClient(client);

	sockets_destroyServer(server);

	return EXIT_SUCCESS;
}

int main_queue(void) {
	t_queue *queue = queue_create(20);

	pthread_t producer, consumer;

	pthread_create(&producer, NULL, produce, queue);
	pthread_create(&consumer, NULL, consume, queue);

	pthread_join(producer, NULL);
	pthread_join(consumer, NULL);

	return EXIT_SUCCESS;
}

void *produce(void* queue){
	t_queue *q = (t_queue *)queue;
	int i=0;
	while (i < 100) {
		int *e = (int*)malloc(sizeof(int));
		*e = i;
		queue_push(q, e);
		printf("enqueue %i\n", i++);
	}
	return NULL;
}

void *consume(void* queue){
	t_queue *q = (t_queue *)queue;
	int *e;
	do {
		e = queue_pop(q);
		printf("dequeue %i\n", *e);
		free(e);
	} while (1);
	return NULL;
}

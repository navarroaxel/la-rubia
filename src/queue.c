#include <stdio.h>
#include <errno.h>
#include "queue.h"

t_queue *queue_create(int size) {
	t_queue *q = (t_queue *)malloc(sizeof(t_queue));
	sem_init(&q->empty, 0, 0);
	sem_init(&q->full, 0, size);
	pthread_mutex_init(&q->mutex, NULL);
	q->head = q->tail = NULL;
	return q;
}

void enqueue(t_queue *q, const void *value) {
	t_queue_node *node = (t_queue_node *)malloc(sizeof(t_queue_node));
    node->data = value;

    sem_wait(&q->full);
    pthread_mutex_lock(&q->mutex);
    if (q->head == NULL)
        q->head = q->tail = node;
    else {
        q->tail->next = node;
        q->head = node;
    }
    node->next = NULL;
    pthread_mutex_unlock(&q->mutex);
    sem_post(&q->empty);
}

void *dequeue(t_queue *q) {
	sem_wait(&q->empty);
	pthread_mutex_lock(&q->mutex);
    void *value = q->head->data;
    t_queue_node *tmp = q->head;
    if (q->head == q->tail)
        q->head = q->tail = NULL;
    else
        q->head = q->tail->next;

    pthread_mutex_unlock(&q->mutex);
    sem_post(&q->full);

    free(tmp);
    return value;
}

void queue_destroy(t_queue *q) {
	sem_destroy(&q->empty);
	sem_destroy(&q->full);
	pthread_mutex_destroy(&q->mutex);
	free(q);
}

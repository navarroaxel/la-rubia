#ifndef QUEUE_C
#define QUEUE_C

#include <semaphore.h>
#include <pthread.h>

typedef struct  {
    void *data;
    struct t_queue_node *next;
} t_queue_node;

typedef struct {
	t_queue_node *head;
    t_queue_node *tail;
    sem_t full;
    sem_t empty;
    pthread_mutex_t mutex;
} t_queue;

t_queue *queue_create(int size);
void queue_push(t_queue *q, const void *data);
void *queue_pop(t_queue *q);
void queue_destroy(t_queue *q);

#endif

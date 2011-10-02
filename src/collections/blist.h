#ifndef BLIST_H_
#define BLIST_H_

#include <stdlib.h>
#include <semaphore.h>
#include "collections.h"

typedef struct{
	t_link_element *head;
	int elements_count;
	sem_t semaphore;
	sem_t full;
	sem_t empty;
} t_blist;

t_blist *collection_blist_create(int maxLength);
void *collection_blist_get(t_blist *list);
void collection_blist_put(t_blist *list, void *data, int (*closure)(void*));
void collection_blist_push(t_blist *list, void *data);
void *collection_blist_pop(t_blist *list);

#endif /*BLIST_H_*/

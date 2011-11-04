#ifndef BLIST_H_
#define BLIST_H_

#include <stdlib.h>
#include <semaphore.h>
#include "collections.h"
#include "list.h"

typedef struct {
	t_link_element *head;
	int elements_count;
	sem_t semaphore;
	sem_t full;
	sem_t empty;
} t_blist;

t_blist *collection_blist_create(int maxLength);
void *collection_blist_get(t_blist *list);
void collection_blist_put(t_blist *list, void *data, int(*closure)(void*));
void collection_blist_push(t_blist *list, void *data);
void *collection_blist_pop(t_blist *list);
void collection_blist_iterator(t_blist *list, void(*closure)(void*));
int collection_blist_size(t_blist *list);
void collection_blist_move(t_blist *blist, t_list *list);
void *collection_blist_popfirst(t_blist *blist, int(*closure)(void *data));
void *collection_blist_getfirst(t_blist *list, int(*closure)(void*));
void collection_blist_destroy(t_blist *list);

#endif /*BLIST_H_*/

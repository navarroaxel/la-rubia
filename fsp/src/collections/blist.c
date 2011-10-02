#include "blist.h"

t_blist *collection_blist_create(int maxLength){
	t_blist *list = (t_blist *)malloc(sizeof(t_blist));
	list->head = NULL;
	list->elements_count = 0;
	if( sem_init( &list->semaphore, 0, 1) == -1 ) {
		free(list);
		return NULL;
	}
	if(sem_init(&list->full, 0, maxLength) == -1) {
		sem_destroy(&list->semaphore);
		free(list);
		return NULL;
	}
	if(sem_init(&list->empty, 0, 0)){
		sem_destroy(&list->semaphore);
		sem_destroy(&list->full);
		free(list);
		return NULL;
	}
	return list;
}

void *collection_blist_pop(t_blist *list) {
	void *data = NULL;

	sem_wait(&list->empty);
	sem_wait( &list->semaphore );

	t_link_element *e = list->head;
	list->head = list->head->next;
	list->elements_count--;
	data=e->data;
	free(e);

	sem_post( &list->semaphore );
	sem_post(&list->full);
	return data;
}

void collection_blist_put(t_blist *list, void *data, int (*closure)(void*)){
	sem_wait(&list->full);
	sem_wait(&list->semaphore );
	t_link_element *new_element = (t_link_element *)malloc(sizeof(t_link_element));
	new_element->data = data;

	t_link_element *prev_element;
	t_link_element *e = list->head;
	while (e != NULL && closure(e->data)){
		prev_element = e;
		e = e->next;
	}
	if (e == list->head) {
		new_element->next = list->head;
		list->head=new_element;
	}
	else {
		prev_element->next = new_element;
		new_element->next = e;
	}
	sem_post( &list->semaphore );
	sem_post(&list->empty);
}

void collection_blist_push(t_blist *list, void *data){
	sem_wait(&list->full);
	sem_wait(&list->semaphore );
	t_link_element *new_element = malloc (sizeof(t_link_element));
	new_element->data = data;

	t_link_element *e = list->head;
	while (e != NULL)
		e = e->next;

	if (e == list->head) {
		new_element->next = list->head;
		list->head = new_element;
	}
	else {
		new_element->next = e;
	}

	sem_post( &list->semaphore );
	sem_post(&list->empty);
}
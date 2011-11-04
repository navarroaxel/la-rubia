#include "blist.h"

t_blist *collection_blist_create(int maxLength) {
	t_blist *list = malloc(sizeof(t_blist));
	list->head = NULL;
	list->elements_count = 0;
	if (sem_init(&list->semaphore, 0, 1) == -1) {
		free(list);
		return NULL;
	}
	if (sem_init(&list->full, 0, maxLength) == -1) {
		sem_destroy(&list->semaphore);
		free(list);
		return NULL;
	}
	if (sem_init(&list->empty, 0, 0)) {
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
	sem_wait(&list->semaphore);

	t_link_element *e = list->head;
	list->head = list->head->next;
	list->elements_count--;
	data = e->data;
	free(e);

	sem_post(&list->semaphore);
	sem_post(&list->full);
	return data;
}

void collection_blist_put(t_blist *list, void *data, int(*closure)(void*)) {
	sem_wait(&list->full);
	sem_wait(&list->semaphore);
	t_link_element *new_element = (t_link_element *) malloc(
			sizeof(t_link_element));
	new_element->data = data;

	t_link_element *prev_element;
	t_link_element *e = list->head;
	while (e != NULL && closure(e->data)) {
		prev_element = e;
		e = e->next;
	}
	if (e == list->head) {
		new_element->next = list->head;
		list->head = new_element;
	} else {
		prev_element->next = new_element;
		new_element->next = e;
	}

	list->elements_count++;

	sem_post(&list->semaphore);
	sem_post(&list->empty);
}

void collection_blist_push(t_blist *list, void *data) {
	sem_wait(&list->full);
	sem_wait(&list->semaphore);
	t_link_element *new_element = malloc(sizeof(t_link_element));
	new_element->data = data;
	new_element->next = NULL;

	if (list->head == NULL
		)
		list->head = new_element;
	else {
		t_link_element *e = list->head;
		while (e->next != NULL)
			e = e->next;

		e->next = new_element;
	}

	list->elements_count++;

	sem_post(&list->semaphore);
	sem_post(&list->empty);
}

void collection_blist_iterator(t_blist *list, void(*closure)(void*)) {
	sem_wait(&list->semaphore);

	t_link_element *e = list->head;
	while (e != NULL) {
		closure(e->data);
		e = e->next;
	}

	sem_post(&list->semaphore);
}

int collection_blist_size(t_blist *list) {
	return list->elements_count;
}

void *collection_blist_popfirst(t_blist *list, int(*closure)(void*)) {
	sem_wait(&list->empty);
	sem_wait(&list->semaphore);

	t_link_element *prev_element;
	t_link_element *e = list->head;
	while (e != NULL && !closure(e->data)) {
		prev_element = e;
		e = e->next;
	}

	if (e == NULL) {
		sem_post(&list->semaphore);
		sem_post(&list->empty);
		return NULL;
	}

	if (e == list->head) {
		list->head = e->next;
	} else {
		prev_element->next = e->next;
	}

	list->elements_count--;

	sem_post(&list->semaphore);
	sem_post(&list->full);

	void *data = e->data;
	free(e);
	return data;
}

void *collection_blist_getfirst(t_blist *list, int(*closure)(void*)) {
	sem_wait(&list->semaphore);

	t_link_element *e = list->head;
	while (e != NULL && !closure(e->data))
		e = e->next;

	if (e == NULL) {
		sem_post(&list->semaphore);
		return NULL;
	}

	void *data = e->data;
	sem_post(&list->semaphore);
	return data;
}

void collection_blist_move(t_blist *blist, t_list *list) {
	sem_wait(&blist->empty);
	sem_wait(&blist->semaphore);
	sem_wait(&list->semaforo);

	if (list->head != NULL) {
		t_link_element *e = list->head;
		while (e->next != NULL)
			e = e->next;

		e->next = blist->head;
	} else
		list->head = blist->head;

	list->elements_count += blist->elements_count--;
	blist->head = NULL;
	sem_post(&blist->full);

	for (; blist->elements_count > 0; blist->elements_count--) {
		sem_wait(&blist->empty);
		sem_post(&blist->full);
	}

	sem_post(&blist->semaphore);
	sem_post(&list->semaforo);
}

void collection_blist_destroy(t_blist *list){
	sem_destroy(&list->empty);
	sem_destroy(&list->full);
	sem_destroy(&list->semaphore);
	free(list);
}

/*
 * @FILE: 		object.c
 * @AUTHOR: 	vialef
 * @CREATED_ON: 16/02/2010
 * @VERSION:	
 */

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "Class.h"
#include "Object.h"

void * new(const void * _class, ...) {
	const struct Class * clazz = _class;
	void * p = calloc(1, clazz->size);
	assert(p);
	*(const struct Class **) p = clazz;
	if (clazz->ctor) {
		va_list ap;
		void * auxp;
		va_start(ap, _class);
		auxp = clazz->ctor(p, &ap);
		va_end(ap);
		if( auxp == NULL ) free(p);
		p = auxp;
	}

	return p;
}

int instanceOf(const void * obj, const void * _class) {
	const struct Class ** cp = obj;
	const struct Class * clazz = _class;
	return strcmp((*cp)->name, clazz->name) == 0 && (*cp)->size == clazz->size;
}

void delete(void * self) {
	const struct Class ** cp = self;
	if (self && *cp && (*cp)->dtor)
		self = (*cp)->dtor(self);
	free(self);
}


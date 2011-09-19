/*
 * @FILE: 		class.c
 * @AUTHOR: 	vialef
 * @CREATED_ON: 16/02/2010
 * @VERSION:	
 */
#include <string.h>

#include "Class.h"

int Class_equals(const void * _class1, const void * _class2) {
	const struct Class * class1 = _class1;
	const struct Class * class2 = _class2;
	return strcmp(class1->name, class2->name) == 0 && class1->size == class2->size;
}

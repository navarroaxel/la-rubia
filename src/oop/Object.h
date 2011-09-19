/*
 * @FILE: 		object.h
 * @AUTHOR: 	vialef
 * @CREATED_ON: 16/02/2010
 * @VERSION:	
 */

#ifndef OBJECT_H_
#define OBJECT_H_

	struct SerializedObject {
		char *bytes;
		size_t size;
	};

	void * new (const void * type, ...);
	int instanceOf (const void * obj, const void * clazz);
	void delete (void * obj);

#endif /* OBJECT_H_ */

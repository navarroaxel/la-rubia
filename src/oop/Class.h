/*
 * @FILE: 		class.h
 * @AUTHOR: 	vialef
 * @CREATED_ON: 16/02/2010
 * @VERSION:	
 */

#ifndef CLASS_H_
#define CLASS_H_

	#include <stddef.h>
	#include <stdarg.h>

	#define extends(type)	type##_DEF
	#define extends2(type1, type2)	type1##_DEF	\
									type2##_DEF

	struct Class {
		const char * name;
		size_t size;
		void * (* ctor) (void * self, va_list * app);
		int    (* equals) (const void * _class1, const void * _class2);
		void * (* dtor) (void * self);
	};

	int Class_equals(const void * _class1, const void * _class2);

#endif /* CLASS_H_ */

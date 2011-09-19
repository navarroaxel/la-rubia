/*
 * SocketAddress.h
 *
 *  Created on: 16/04/2010
 *      Author: facundoviale
 */

#ifndef SOCKETADDRESS_H_
#define SOCKETADDRESS_H_

	#define _XOPEN_SOURCE 500

	#include <sys/select.h>
	#include <netinet/in.h>

	#include "../oop/Class.h"


	struct SocketAddress{
		const void * clazz;
		/* Native Address */
		struct sockaddr_in nAddress;
		char ip[16];
		int port;
	};

	struct SocketAddress* SocketAddress_constructor( struct SocketAddress*, va_list *args );
	void*		 		  SocketAddress_destroyer( struct SocketAddress* );

	static const struct Class _SocketAddress = {
		"SocketAddress",
		sizeof(struct SocketAddress),
		(void*(*)(void*,va_list*))SocketAddress_constructor, Class_equals, (void*(*)(void*))SocketAddress_destroyer
	};

	static const void * SocketAddress = &_SocketAddress;

#endif /* SOCKETADDRESS_H_ */

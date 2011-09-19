/*
 * SocketServer.h
 *
 *  Created on: 24/04/2010
 *      Author: facundoviale
 */

#ifndef SOCKETSERVER_H_
#define SOCKETSERVER_H_

	#define _XOPEN_SOURCE 500

	#include "../oop/Class.h"

	#include "../collections/List.h"

	#include "Socket.h"
	#include "SocketClient.h"

	#define DEFAULT_MAX_CONEXIONS		100
	#define DEFAULT_SELECT_TIMEOUT		0

	struct SocketServerClass{
		int 				(*listen)(struct SocketServer *);
		struct SocketClient *(*accept)(struct SocketServer *);
		int 				(*select1)(struct SocketServer *, struct List *clients,
															  struct SocketClient * (*onAccept)(struct SocketServer *),
															  int (*onRecive)(struct SocketClient *)  );
	};

	struct SocketServer *SocketServer_constructor( struct SocketServer*, va_list *args );
	int			  		 SocketServer_listen(struct SocketServer *);
	struct SocketClient *SocketServer_accept(struct SocketServer *);
	int  				 SocketServer_select1(struct SocketServer *, struct List *clients,
																	 struct SocketClient * (*onAccept)(struct SocketServer *),
																	 int (*onRecive)(struct SocketClient *) );
	void*		 		 SocketServer_destroyer( struct SocketServer* );

	static const struct Class _SocketServer = {
		"SocketServer",
		sizeof(struct SocketServer),
		(void*(*)(void*,va_list*))SocketServer_constructor, Class_equals, (void*(*)(void*))SocketServer_destroyer
	};

	static const void * SocketServer = &_SocketServer;

	static const struct SocketServerClass SocketServerClass = {
			SocketServer_listen,
			SocketServer_accept,
			SocketServer_select1
	};

#endif /* SOCKETSERVER_H_ */

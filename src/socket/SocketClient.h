/*
 * SocketClient.h
 *
 *  Created on: 16/04/2010
 *      Author: facundoviale
 */

#ifndef SOCKETCLIENT_H_
#define SOCKETCLIENT_H_

	#define _XOPEN_SOURCE 500

	#include "../oop/Class.h"
	#include "../oop/Object.h"

	#include "Socket.h"

	#define DEFAULT_RECIVE_BUFFER_SIZE		2048
	#define DEFAULT_NUMBER_OF_SEND_RETRYS	10

	struct SocketClientClass{
		int (*connect)(struct SocketClient *, char *ip, int port);
		int (*send1)(struct SocketClient *, char *data, int datalength);
		int (*send2)(struct SocketClient *, char *string);
		int (*send3)(struct SocketClient *, void *data, struct SerializedObject *(*serializer)(void *data));
		int (*recive1)(struct SocketClient *, char *buffer, int buffersize);
		void *(*recive2)(struct SocketClient *, void *(*deserializer)(void *buffer, int buffersize));
	};

	struct SocketClient* SocketClient_constructor( struct SocketClient*, va_list *args );
	int			  		 SocketClient_connect(struct SocketClient *, char *ip, int port);
	int  				 SocketClient_send1(struct SocketClient *, char *data, int datalength);
	int  				 SocketClient_send2(struct SocketClient *, char *string);
	int 				 SocketClient_send3(struct SocketClient *, void *data, struct SerializedObject *(*serializer)(void *data));
	int  				 SocketClient_recive1(struct SocketClient *, char *buffer, int buffersize);
	void  				*SocketClient_recive2(struct SocketClient *, void *(*deserializer)(void *buffer, int buffersize));
	void*		 		 SocketClient_destroyer( struct SocketClient* );

	static const struct Class _SocketClient = {
		"SocketClient",
		sizeof(struct SocketClient),
		(void*(*)(void*,va_list*))SocketClient_constructor, Class_equals, (void*(*)(void*))SocketClient_destroyer
	};

	static const void * SocketClient = &_SocketClient;

	static const struct SocketClientClass SocketClientClass = {
			SocketClient_connect,
			SocketClient_send1,
			SocketClient_send2,
			SocketClient_send3,
			SocketClient_recive1,
			SocketClient_recive2
	};

#endif /* SOCKETCLIENT_H_ */

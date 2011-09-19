/*
 * Socket.h
 *
 *  Created on: 16/04/2010
 *      Author: facundoviale
 */

#ifndef SOCKET_H_
#define SOCKET_H_

	#define _XOPEN_SOURCE 500


	#include <sys/select.h>
	#include <netinet/in.h>

	#include "../oop/Class.h"

	#include "SocketAddress.h"

	#define DEFAULT_BUFFER_SIZE 	2048
	#define DEFAULT_MAX_CONEXIONS 	100
	#define SELECT_USEC_TIMEOUT 	500

	typedef enum {
		SOCKET_STATE_CONNECTED		= 1,
		SOCKET_STATE_DISCONNECTED	= 2
	}SocketState;

	typedef enum {
		SOCKET_MODE_NONBLOCK		= 1,
		SOCKET_MODE_BLOCK			= 2
	}SocketMode;

	struct Socket{
	#define Socket_DEF 					\
		const void * clazz;				\
		int descriptor;					\
		struct SocketAddress *address;	\
		SocketMode mode;
	Socket_DEF
	};

	struct SocketClient {	extends(Socket)
		struct SocketAddress *server_address;
		SocketState state;
	};

	struct SocketServer {	extends(Socket)
		int maxconexions;
		int select_usec_timeout;
	};

#endif /* SOCKET_H_ */

/*
 * Copyright (c) 2011, C-Talks and/or its owners. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact C-Talks owners, Matias Dumrauf or Facundo Viale
 * or visit https://sites.google.com/site/utnfrbactalks/ if you need additional
 * information or have any questions.
 */

/*
 *	@FILE: 	sockets.h
 *	@AUTOR: Facundo Viale
 *	@VERSION:	2.0b1 (12/09/2009)
 *					- First Release
 */

#ifndef OLD_SOCKETS_H_
#define OLD_SOCKETS_H_


	#define _XOPEN_SOURCE 500


	#include <sys/select.h>
	#include <netinet/in.h>
	#include "collections/list.h"

	#define DEFAULT_BUFFER_SIZE 	2048
	#define DEFAULT_MAX_CONEXIONS 	100
	#define SELECT_USEC_TIMEOUT 	500

	typedef enum {
		SOCKETSTATE_CONNECTED,
		SOCKETSTATE_DISCONNECTED
	}e_socket_state;

	typedef enum {
		SOCKETMODE_NONBLOCK		= 1,
		SOCKETMODE_BLOCK		= 2
	}e_socket_mode;


	typedef struct {
		int desc;
		struct sockaddr_in* my_addr;
		e_socket_mode mode;
	} t_socket ;

	typedef struct {
		t_socket* socket;
		t_socket* serv_socket;
		e_socket_state state;
	} t_socket_client ;

	typedef struct {
		t_socket    *socket;
		int			maxconexions;
	} t_socket_server ;

	typedef struct {
		char data[DEFAULT_BUFFER_SIZE];
		int size;
	} t_socket_buffer ;

	typedef struct {
		void *serializated_data;
		int size;
	} t_socket_sbuffer ;


	void			sockets_bufferDestroy(t_socket_buffer *tbuffer);
	void			sockets_sbufferDestroy(t_socket_sbuffer *tbuffer);

	t_socket        *sockets_getClientSocket(t_socket_client *client);
	t_socket        *sockets_getServerSocket(t_socket_server *server);

	char            *sockets_getIp(t_socket *sckt);
	void             sockets_getIpAsBytes(t_socket *sckt, unsigned char ip[]);
	void             sockets_getStringIpAsBytes(char *str_ip, unsigned char ip[]);
	int              sockets_getPort(t_socket *sckt);

	void             sockets_setMode(t_socket *sckt, e_socket_mode mode);
	e_socket_mode    sockets_getMode(t_socket *sckt);
	int              sockets_isBlocked(t_socket *sckt);

	t_socket_client *sockets_createClient(char *ip, int port);
	int				 sockets_isConnected(t_socket_client *client);
	int              sockets_equalsClients(t_socket_client *client1, t_socket_client *client2);
	e_socket_state	 sockets_getState(t_socket_client *client);
	void			 sockets_setState(t_socket_client *client, e_socket_state state);
	int              sockets_connect(t_socket_client *client, char *server_ip, int server_port);
	int              sockets_send(t_socket_client *client, void *data, int datalen);
	int              sockets_sendBuffer(t_socket_client *client, t_socket_buffer *buffer);
	int              sockets_sendSBuffer(t_socket_client *client, t_socket_sbuffer *buffer);
	int              sockets_sendString(t_socket_client *client, char *str);
	int              sockets_sendSerialized(t_socket_client *client, void *data, t_socket_sbuffer *(*serializer)(void*));
	t_socket_buffer *sockets_recv(t_socket_client *client);
	int 			 sockets_recvInBuffer(t_socket_client *client, t_socket_buffer *buffer);
	int 			 sockets_recvInSBuffer(t_socket_client *client, t_socket_sbuffer *buffer);
	void            *sockets_recvSerialized(t_socket_client *client, void *(*deserializer)(t_socket_sbuffer*));
	void             sockets_destroyClient(t_socket_client *client);

	t_socket_server *sockets_createServer(char *ip, int port);
	void 			 sockets_setMaxConexions(t_socket_server* server, int conexions);
	int              sockets_getMaxConexions(t_socket_server* server);
	int 			 sockets_listen(t_socket_server* server);
	t_socket_client *sockets_accept(t_socket_server* server);
	void 			 sockets_select(t_list* servers,
									t_list *clients,
									int usec_timeout,
									t_socket_client *(*onAcceptClosure)(t_socket_server*),
									int (*onRecvClosure)(t_socket_client*) );
	void 			 sockets_destroyServer(t_socket_server* server);

#endif /* OLD_SOCKETS_H_ */

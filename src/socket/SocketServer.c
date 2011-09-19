#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <assert.h>

#include "../oop/Object.h"

#include "SocketServer.h"

struct SocketServer *SocketServer_constructor( struct SocketServer *self, va_list *args ){
	int yes=1;

	assert( (self->descriptor = socket(AF_INET, SOCK_STREAM, 0)) > 2 );

	{
		char *ip = va_arg(* args, char*);
		int port = va_arg(* args, int);
		self->address = new(SocketAddress, ip, port);
	}

	assert( setsockopt(self->descriptor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) != -1 );

	assert( bind(self->descriptor, (struct sockaddr *)&(self->address->nAddress), sizeof(struct sockaddr_in)) != -1 );

	self->maxconexions = DEFAULT_MAX_CONEXIONS;

	self->select_usec_timeout = DEFAULT_SELECT_TIMEOUT;

	return self;
}

int SocketServer_listen(struct SocketServer *self){
	return listen(self->descriptor, self->maxconexions) == 0;
}


struct SocketClient *SocketServer_accept(struct SocketServer *self){
	struct SocketClient* client = malloc( sizeof(struct SocketClient) );
	client->address = malloc( sizeof(struct SocketAddress) );
	int addrlen = sizeof(struct sockaddr_in);

	if( self->mode == SOCKET_MODE_NONBLOCK ){
		fcntl(self->descriptor, F_SETFL, O_NONBLOCK);
	}

	if ((client->descriptor = accept(self->descriptor, (struct sockaddr *)&(client->address->nAddress), (void *)&addrlen)) == -1) {
		free(client->address);
		free(client);
		return NULL;
	}

	if( self->mode == SOCKET_MODE_NONBLOCK ){
		fcntl(self->descriptor, F_SETFL, O_NONBLOCK);
	}

	client->state = SOCKET_STATE_CONNECTED;
	client->mode = SOCKET_MODE_BLOCK;

	return client;
}

int SocketServer_select1(struct SocketServer *self, struct List *clients, struct SocketClient * (*onAccept)(struct SocketServer *), int (*onRecive)(struct SocketClient *) ){
	fd_set conexions_set;
	int max_desc = 3;
	int select_return;
	struct timeval tv;
	struct List *closed_clients = NULL;

	tv.tv_sec = 0;
	tv.tv_usec = self->select_usec_timeout;

	if( clients == NULL || ListClass.size(clients) == 0 )	return 1;


	void innerclosure_clientsBuildSet(struct SocketClient *client){
		int curr_desc = client->descriptor;
		FD_SET(curr_desc, &conexions_set);
		if( max_desc < curr_desc ) max_desc = curr_desc;
	}

	ListClass.iterator(clients, (void*)&innerclosure_clientsBuildSet);

	FD_SET(self->descriptor, &conexions_set);
	if( max_desc < self->descriptor ) max_desc = self->descriptor;

	if( self->select_usec_timeout == 0 ){
		select_return = select( max_desc + 1, &conexions_set, NULL, NULL, NULL );
	}else{
		select_return = select( max_desc + 1, &conexions_set, NULL, NULL, &tv );
	}

	if( select_return == -1 ) return 0;

	if( FD_ISSET(self->descriptor, &conexions_set) ){
		struct SocketClient *newClient = NULL;
		if( &onAccept != NULL){
			newClient = onAccept(self);
		} else {
			newClient = SocketServer_accept(self);
		}

		ListClass.add(clients, newClient);
    }

	void innerclosure_reciveFromSocket(struct SocketClient *client){
		if( FD_ISSET(client->descriptor, &conexions_set) && (&onRecive != NULL) ){
			if( onRecive(client) == 0 ){
				if( closed_clients == NULL )	closed_clients = new(List);
				ListClass.add(closed_clients, client);
	    	}
		}
	}

	ListClass.iterator(clients, (void*)&innerclosure_reciveFromSocket);

	if( closed_clients != NULL ){

		int innerclosure_removeSocket(struct SocketClient *aux){
			ListClass.removeByPointer(clients, aux);
			return 1;
		}

		ListClass.removeByClosure1(closed_clients, (void*)&innerclosure_removeSocket);

	    delete(closed_clients);
	}

	return 1;
}

void* SocketServer_destroyer( struct SocketServer *self ){
	delete(self->address);
	return self;
}


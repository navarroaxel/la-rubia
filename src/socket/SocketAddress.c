#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

#include "SocketAddress.h"

static void SocketAddress_make( struct SocketAddress *self );

struct SocketAddress* SocketAddress_constructor( struct SocketAddress *self, va_list *args ){
	char* ip = va_arg(* args, char*);

	if( ip != NULL ){
		strcpy(self->ip, ip);
		self->port = va_arg(* args, int);
	} else {
		strcpy(self->ip, "127.0.0.1");
		self->port = 0;
	}

	SocketAddress_make( self );

	return self;
}

static void SocketAddress_make( struct SocketAddress *self ){

	self->nAddress.sin_family = AF_INET;

	self->nAddress.sin_addr.s_addr=inet_addr( self->ip );

	self->nAddress.sin_port = htons( self->port );
}

void*	SocketAddress_destroyer( struct SocketAddress *self){
	return self;
}

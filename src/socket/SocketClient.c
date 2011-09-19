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

#include "SocketClient.h"


struct SocketClient* SocketClient_constructor( struct SocketClient *self, va_list *args ){
	int yes=1;

	assert( (self->descriptor = socket(AF_INET, SOCK_STREAM, 0)) > 2 );

	{
		char *ip = va_arg(* args, char*);
		int port = va_arg(* args, int);
		self->address = new(SocketAddress, ip, port);
	}

	assert( setsockopt(self->descriptor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) != -1 );

	assert( bind(self->descriptor, (struct sockaddr *)&(self->address->nAddress), sizeof(struct sockaddr_in)) != -1 );

	self->state = SOCKET_STATE_DISCONNECTED;
	return self;
}

int	SocketClient_connect(struct SocketClient *self, char *ip, int port){
	struct SocketAddress *server_address = new(SocketAddress, ip, port);

	if ( connect(self->descriptor, (void*)&(server_address->nAddress), sizeof(struct sockaddr_in) ) == -1) {
		delete(server_address);
		return 0;
	}

	self->server_address = server_address;

	self->state = SOCKET_STATE_CONNECTED;

	return 1;
}

int	SocketClient_send1(struct SocketClient *self, char *data, int datalength){
	int retrys;

	for(retrys = 0 ; retrys < DEFAULT_NUMBER_OF_SEND_RETRYS; retrys++){
		int aux = send(self->descriptor, data, datalength, 0);
		if( aux == -1) break;
		if( aux == datalength) return 1;
	}

	self->state = SOCKET_STATE_DISCONNECTED;

	return 0;
}

int	SocketClient_send2(struct SocketClient *self, char *string){
	return SocketClient_send1(self, string, strlen(string) + 1);
}

int SocketClient_send3(struct SocketClient *self, void *data, struct SerializedObject *(*serializer)(void *data)){
	struct SerializedObject *stream = serializer(data);

	int ret = SocketClient_send1(self, stream->bytes, stream->size);

	free(stream->bytes);
	free(stream);

	return ret;
}

int	SocketClient_recive1(struct SocketClient *self, char *buffer, int buffersize){
	int rcv_bytes;
	memset(buffer, 0, buffersize);

	if( self->state == SOCKET_MODE_NONBLOCK ){
		fcntl(self->descriptor, F_SETFL, O_NONBLOCK);
	}

	rcv_bytes = recv(self->descriptor, buffer, buffersize, 0);

	if( self->state == SOCKET_MODE_NONBLOCK  ){
		fcntl(self->descriptor, F_SETFL, O_NONBLOCK);
	}

	if( rcv_bytes == -1 )
		self->state = SOCKET_STATE_DISCONNECTED;

	return rcv_bytes;
}

void *SocketClient_recive2(struct SocketClient *self, void *(*deserializer)(void *buffer, int buffersize)){
	char buffer[DEFAULT_RECIVE_BUFFER_SIZE];
	int rcv_bytes = SocketClient_recive1(self, buffer, DEFAULT_RECIVE_BUFFER_SIZE);

	if( rcv_bytes > 0 ){
		return deserializer(buffer, rcv_bytes);
	}
	return NULL;
}

void *SocketClient_destroyer( struct SocketClient *self ){
	delete(self->address);
	return self;
}


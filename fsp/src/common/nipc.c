#include "nipc.h"
#include <assert.h>

t_nipc *nipc_create(uint8_t type) {
	t_nipc *nipc = (t_nipc *) malloc(sizeof(t_nipc));
	nipc->type = type;
	nipc->length = 0;
	nipc->payload = NULL;

	return nipc;
}

void nipc_setdata(t_nipc *nipc, void *data, uint16_t length) {
	nipc->length = length;
	nipc->payload = data;
}

void *nipc_getdata(t_nipc *nipc) {
	return nipc->payload;
}

void nipc_destroy(t_nipc *nipc) {
	free(nipc->payload);
	free(nipc);
}

t_socket_buffer *nipc_serializer(t_nipc *nipc) {
	int tmpsize, offset = 0;
	t_socket_buffer *buffer = malloc(sizeof(t_socket_buffer));

	memcpy(buffer->data, &nipc->type, tmpsize = sizeof(uint8_t));
	offset += tmpsize;

	memcpy(buffer->data + offset, &nipc->length, tmpsize = sizeof(uint16_t));
	offset += tmpsize;

	memcpy(buffer->data + offset, nipc->payload, nipc->length);
	buffer->size = offset + nipc->length;

	return buffer;
}

t_nipc *nipc_deserializer(t_socket_buffer *buffer, uint32_t offsetinBuffer) {
	int tmpsize, offset;
	t_nipc *nipc = malloc(sizeof(t_nipc));

	memcpy(&nipc->type, buffer->data, offset = sizeof(uint8_t));
	memcpy(&nipc->length, buffer->data + offset, tmpsize = sizeof(uint16_t));
	offset += tmpsize;

	nipc->payload = malloc(nipc->length);
	memcpy(nipc->payload, buffer->data + offset, nipc->length);

	return nipc;
}

t_nipc *nipc_clone(t_nipc *nipc) {
	t_nipc *new = malloc(sizeof(t_nipc));
	new->type = nipc->type;
	new->length = nipc->length;

	new->payload = malloc(sizeof(nipc->length));
	memcpy(new->payload, nipc->payload, nipc->length);

	return new;
}

void nipc_send(t_nipc *nipc, t_socket_client *client){
	t_socket_buffer *buffer = nipc_serializer(nipc);
	sockets_sendBuffer(client, buffer);
	sockets_bufferDestroy(buffer);
}

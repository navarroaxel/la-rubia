#include "nipc.h"

t_nipc *nipc_create(uint8_t type){
	t_nipc *nipc = (t_nipc *)malloc(sizeof(t_nipc));
	nipc->type = type;
	nipc->length = 0;
	nipc->payload = NULL;

	return nipc;
}

void nipc_setdata(t_nipc *nipc, void *data, uint16_t length){
	nipc->length = length;
	nipc->payload = data;
}

void *nipc_getdata(t_nipc *nipc){
	return nipc->payload;
}

void nipc_destroy(t_nipc *nipc){
	free(nipc);
}

void *nipc_getdata_destroy(t_nipc *nipc){
	void *data = nipc_getdata(nipc);
	nipc_destroy(nipc);
	return data;
}

t_socket_buffer *nipc_serializer(t_nipc *nipc) {
	int tmpsize, offset = 0;
	t_socket_buffer *stream = (t_socket_buffer *)malloc(sizeof(t_socket_buffer));

	memcpy(stream->data, &nipc->type, tmpsize = sizeof(uint8_t));
	offset += tmpsize;

	memcpy(stream->data + offset, &nipc->length, tmpsize = sizeof(uint16_t));
	offset += tmpsize;

	memcpy(stream->data + offset, nipc->payload, nipc->length);
	stream->size = offset + nipc->length;

	return stream;
}

t_nipc *nipc_deserializer(t_socket_buffer *buffer){
	int tmpsize, offset = 0;
	t_nipc *nipc = malloc(sizeof(t_nipc));

	memcpy(&nipc->type, buffer->data, tmpsize = sizeof(uint8_t));
	offset += tmpsize;

	memcpy(&nipc->length, buffer->data + offset, tmpsize = sizeof(uint16_t));
	offset += tmpsize;

	nipc->payload = malloc(nipc->length);
	memcpy(nipc->payload, buffer->data + offset, nipc->length);

	return nipc;
}

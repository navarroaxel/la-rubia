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


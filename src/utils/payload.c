#include "payload.h"
#include "stdlib.h"

typedef struct {
	uint8_t type;
	uint16_t length;
	void *payload;
} t_nipc;

void nipc_setdata(t_nipc *nipc, void *data, uint16_t length){
	nipc->length = length;
	nipc->payload = data;
}

void nipc_create(t_nipc *nipc, uint8_t type){
	nipc->type = type;
	nipc->length = 0;
	nipc->payload = NULL;
}

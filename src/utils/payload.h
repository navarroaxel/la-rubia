#ifndef PAYLOAD_C
#define PAYLOAD_C

#include <stdint.h>

#define NIPC_HANDSHAKE_TYPE = 0;

typedef struct t_nipc {
	uint8_t type;
	uint16_t length;
	void *payload;
} t_nipc;

void nipc_create(t_nipc *nipc, uint8_t type);
void nipc_setdata(t_nipc *nipc, void *data, uint16_t length);

#endif

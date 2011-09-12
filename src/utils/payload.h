#ifndef PAYLOAD_C
#define PAYLOAD_C

#define NIPC_HANDSHAKE_TYPE = 0;

typedef t_nipc;

void nipc_create(t_nipc *nipc, uint8_t type);
void nipc_setdata(t_nipc *nipc, void *data, uint16_t length);

#endif

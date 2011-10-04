#ifndef NIPC_H
#define NIPC_H

#include <stdint.h>
#include <stdlib.h>
#include "utils/sockets.h"
#include <string.h>

#define NIPC_HANDSHAKE 0
#define NIPC_READSECTOR_RQ 1
#define NIPC_WRITESECTOR_RQ 2
#define NIPC_READSECTOR_RS 3
#define NIPC_WRITESECTOR_RS 4

#define DISK_RESULT_SUCCESS 0
#define DISK_RESULT_FAILED 1
#define DISK_SECTOR_SIZE 512

typedef struct t_stream {
	int length;
	void *data;
} t_stream;

typedef struct t_nipc {
	uint8_t type;
	uint16_t length;
	void *payload;
} __attribute__ ((packed)) t_nipc;

typedef struct t_disk_readSectorRq {
	uint32_t offset;
} __attribute__ ((packed)) t_disk_readSectorRq;

typedef struct t_disk_writeSectorRq {
	uint32_t offset;
	uint8_t data[512];
} __attribute__ ((packed)) t_disk_writeSectorRq;

typedef struct t_disk_readSectorRs {
	uint8_t result;
	uint32_t offset;
	uint8_t data[512];
} __attribute__ ((packed)) t_disk_readSectorRs;

typedef struct t_disk_writeSectorRs {
	uint8_t result;
	uint32_t offset;
} __attribute__ ((packed)) t_disk_writeSectorRs;

t_nipc *nipc_create(uint8_t type);
void nipc_setdata(t_nipc *nipc, void *data, uint16_t length);
void *nipc_getdata(t_nipc *nipc);
void nipc_destroy(t_nipc *nipc);
void *nipc_getdata_destroy(t_nipc *nipc);
t_socket_buffer *nipc_serializer(t_nipc *nipc);
t_nipc *nipc_deserializer(t_socket_buffer *buffer);

#endif

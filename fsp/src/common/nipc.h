#ifndef NIPC_H
#define NIPC_H

#include <stdint.h>
#include <stdlib.h>

#define NIPC_HANDSHAKE 0
#define NIPC_READSECTOR_RQ 1
#define NIPC_WRITESECTOR_RQ 2
#define NIPC_READSECTOR_RS 3
#define NIPC_WRITESECTOR_RS 4

#define DISK_RESULT_SUCCESS 0
#define DISK_RESULT_FAILED 1

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

#endif


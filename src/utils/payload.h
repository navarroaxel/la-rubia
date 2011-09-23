#ifndef PAYLOAD_C
#define PAYLOAD_C

#include <stdint.h>

#define NIPC_HANDSHAKE 0
#define NIPC_READSECTOR_RQ 1
#define NIPC_WRITESECTOR_RQ 2
#define NIPC_READSECTOR_RS 3
#define NIPC_WRITESECTOR_RS 4

#define DISK_RESULT_SUCCESS 0
#define ISK_RESULT_FAILED 1

typedef struct t_nipc {
	uint8_t type;
	uint16_t length;
	void *payload;
} __attribute__ ((packed)) t_nipc;

typedef struct t_disk_readSectorRq {
	uint32_t offset;
} t_disk_readSectorRq;

typedef struct t_disk_writeSectorRq {
	uint32_t offset;
	uint8_t data[512];
} t_disk_writeSectorRq;

typedef struct t_disk_readSectorRs {
	uint8_t result;
	uint32_t offset;
	uint8_t data[512];
} t_disk_readSectorRs;

typedef struct t_disk_writeSectorRs {
	uint8_t result;
	uint32_t offset;
} t_disk_writeSectorRs;

void nipc_create(t_nipc *nipc, uint8_t type);
void nipc_setdata(t_nipc *nipc, void *data, uint16_t length);

#endif

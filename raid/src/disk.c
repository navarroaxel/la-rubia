#include "disk.h"

void *disk(void *args) {
	struct t_disk *d = (struct t_disk *) args;

	t_socket_buffer *buffer;
	t_nipc *nipc;
	uint32_t offset;

	int findrequest(void *data) {
		return offset == getoffset((t_nipc *)((struct t_fsrq *)data)->nipc);
	}

	while (true) {
		buffer = sockets_recv(d->client);
		nipc = nipc_deserializer(buffer);

		offset = getoffset(nipc);

		struct fsrq * rq = collection_blist_popfirst(d->waiting, findrequest);
	}

	return NULL;
}

int getoffset(t_nipc *nipc) {
	switch (nipc->type) {
	case NIPC_READSECTOR_RQ:
		return ((t_disk_readSectorRq *) nipc->payload)->offset;
	case NIPC_WRITESECTOR_RQ:
		return ((t_disk_writeSectorRq *) nipc->payload)->offset;
	case NIPC_READSECTOR_RS:
		return ((t_disk_readSectorRs *) nipc->payload)->offset;
	case NIPC_WRITESECTOR_RS:
		return ((t_disk_writeSectorRs *) nipc->payload)->offset;
	default:
		return -1;
	}
}

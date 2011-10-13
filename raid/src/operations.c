#include "operations.h"

t_operation *operation_create(t_nipc *nipc) {
	t_operation *op = malloc(sizeof(t_operation));
	switch (nipc->type) {
	case NIPC_READSECTOR_RQ: {
		t_disk_readSectorRq *readrq = (t_disk_readSectorRq *) nipc->payload;
		op->read = true;
		op->offset = readrq->offset;
	}
		break;
	case NIPC_WRITESECTOR_RQ: {
		t_disk_writeSectorRq *writerq = (t_disk_writeSectorRq *) nipc->payload;
		op->read = false;
		op->offset = writerq->offset;
		memcpy(op->data, writerq->data, DISK_SECTOR_SIZE);
	}
		break;
	}

	op->disk = 0;
	return op;
}

void operation_destroy(t_operation *operation) {
	if (operation->client != NULL
	)
		sockets_destroyClient(operation->client);

	free(operation);
}

t_nipc *operation_getnipc(t_operation *op) {
	t_nipc *nipc;
	if (op->read) {
		t_disk_readSectorRq *rq = malloc(sizeof(t_disk_readSectorRq));
		rq->offset = op->offset;
		nipc = nipc_create(NIPC_READSECTOR_RQ);
		nipc_setdata(nipc, rq, sizeof(t_disk_readSectorRq));
	} else {
		t_disk_writeSectorRq *rq = malloc(sizeof(t_disk_writeSectorRq));
		rq->offset = op->offset;
		nipc = nipc_create(NIPC_WRITESECTOR_RQ);
		memcpy(rq->data, op->data, DISK_SECTOR_SIZE);
	}
	return nipc;
}

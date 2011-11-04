#include "disk.h"

void *disk(void *args) {
	t_disk *dsk = (t_disk *) args;

	t_socket_buffer *buffer;
	t_nipc *nipc;
	while (true) {
		buffer = sockets_recv(dsk->client);
		if (buffer == NULL) {
			log_info(dsk->log, dsk->name, "El disco %s se ha desconectado", dsk->name);
			reallocateoperations(dsk);
			return NULL;
		}

		dsk->pendings--;
		nipc = nipc_deserializer(buffer, 0);
		sockets_bufferDestroy(buffer);

		switch (nipc->type) {
		case NIPC_READSECTOR_RS:
			processReadRs(dsk, nipc);
			break;
		case NIPC_WRITESECTOR_RS:
			processWriteRs(dsk, nipc);
			break;
		}
		nipc_destroy(nipc);
	}

	return NULL;
}

void processReadRs(t_disk *d, t_nipc *nipc) {
	t_disk_readSectorRs *rs = (t_disk_readSectorRs *) nipc->payload;

	int findoperation(void *data) {
		t_operation *op = (t_operation *) data;
		return op->read && op->offset == rs->offset && op->disk == d->id;
	}

	t_operation *op = collection_list_popfirst(d->operations, findoperation);

	if (op->client == NULL){
		rs = malloc(sizeof(t_disk_readSectorRs));
		memcpy(rs, nipc->payload, sizeof(t_disk_readSectorRs));
		collection_blist_push(op->syncqueue, rs);
		return;
	}

	nipc_send(nipc, op->client);
	operation_destroy(op);
}

void processWriteRs(t_disk *d, t_nipc *nipc) {
	t_disk_writeSectorRs *rs = (t_disk_writeSectorRs *) nipc->payload;
	bool operationReady = false;
	void findrequest(void *data) {
		t_operation *op = (t_operation *) data;
		if (!op->read && op->offset == rs->offset && (op->disk & d->id) != 0) {
			op->disk ^= d->id;
			operationReady = op->disk == 0;
		}
	}

	int findoperation(void *data) {
		t_operation *op = (t_operation *) data;
		return !op->read && op->offset == rs->offset && op->disk == 0;
	}

	collection_list_iterator(d->operations, findrequest);
	if (operationReady) {
		t_operation *op = collection_list_popfirst(d->operations, findoperation);
		nipc_send(nipc, op->client);
		operation_destroy(op);
	}
}

void reallocateoperations(t_disk *dsk) {
	disks_remove(dsk);
	disks_verifystate();

	void itin(void *data) {
		t_operation *op = (t_operation *) data;
		if (!op->read)
			op->disk &= ~dsk->id;
		else if (op->disk == dsk->id) {
			t_disk *d = disks_getidledisk(op->offset);
			d->pendings++;
			t_nipc *nipc = operation_getnipc(op);
			nipc_send(nipc, d->client);
			nipc_destroy(nipc);
			op->disk = d->id;
		}
	}

	collection_list_iterator(dsk->operations, itin);

	disks_destroy(dsk);
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

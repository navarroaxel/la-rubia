#include "disk.h"

void *disk(void *args) {
	t_disk *dsk = (t_disk *) args;

	t_socket_buffer *buffer;
	t_nipc *nipc;
	uint32_t offsetInBuffer;
	while (true) {
		buffer = sockets_recv(dsk->client);
		offsetInBuffer = 0;
		if (buffer == NULL) {
			log_info(dsk->log, dsk->name, "El disco %s se ha desconectado",
					dsk->name);
			reallocateoperations(dsk);
			return NULL;
		}
		while (offsetInBuffer < buffer->size) {
			disk_decreasepending(dsk);
			nipc = nipc_deserializer(buffer, offsetInBuffer);
			offsetInBuffer += nipc->length + sizeof(nipc->type)
					+ sizeof(nipc->length);
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
		sockets_bufferDestroy(buffer);
	}

	return NULL;
}

void log_clientlost(t_log *logFile, char *thread_name, bool read,
		uint32_t offset) {
	log_warning(logFile, thread_name,
			"Se perdio cliente de operacion de %s offset %i",
			read ? "lectura" : "escritura", offset);
}

void log_lostoperation(t_log *logFile, char *thread_name, bool read,
		uint32_t offset) {
	log_warning(logFile, thread_name, "Se perdio operacion de %s offset %i",
			read ? "lectura" : "escritura", offset);
}

void processReadRs(t_disk *d, t_nipc *nipc) {
	t_disk_readSectorRs *rs = (t_disk_readSectorRs *) nipc->payload;

	int findoperation(void *data) {
		t_operation *op = (t_operation *) data;
		return op->read && op->offset == rs->offset && op->disk == d->id;
	}

	t_operation *op = collection_list_popfirst(d->operations, findoperation);
	if (op == NULL) {
		log_lostoperation(d->log, d->name, true, rs->offset);
		return;
	}

	if (op->client == NULL) {
		if (op->syncqueue == NULL) {
			log_clientlost(d->log, d->name, op->read, rs->offset);
			operation_destroy(op);
			return;
		}
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

	//TODO validar si se encontro la operacion.

	int findoperation(void *data) {
		t_operation *op = (t_operation *) data;
		return !op->read && op->offset == rs->offset && op->disk == 0;
	}

	collection_list_iterator(d->operations, findrequest);
	if (operationReady) {
		t_operation *op = collection_list_popfirst(d->operations,
				findoperation);
		if (op == NULL) {
			log_lostoperation(d->log, d->name, false, rs->offset);
			return;
		}
		if (op->client == NULL) {
			if (op->syncqueue == NULL) {
				log_clientlost(d->log, d->name, op->read, op->offset);
				operation_destroy(op);
				return;
			}
			rs = malloc(sizeof(t_disk_writeSectorRs));
			memcpy(rs, nipc->payload, sizeof(t_disk_writeSectorRs));
			collection_blist_push(op->syncqueue, rs);
			return;
		}
		nipc_send(nipc, op->client);
		operation_destroy(op);
	}
}

void reallocateoperations(t_disk *dsk) {
	disks_remove(dsk);
	disks_verifystate();

	void updateOperationState(void *data) {
		t_operation *op = (t_operation *) data;
		if (!op->read) {
			op->disk &= ~dsk->id;
		} else if (op->disk == dsk->id) {
			t_disk *d = disks_getidledisk(op->offset);
			disk_increasepending(d);
			t_nipc *nipc = operation_getnipc(op);
			nipc_send(nipc, d->client);
			nipc_destroy(nipc);
			op->disk = d->id;
		}
	}

	collection_list_iterator(dsk->operations, updateOperationState);

	while (true) {
		int getReadyWriteOperations(void *data) {
			t_operation *op = data;
			return !op->read && op->disk == 0;
		}

		t_operation *op = collection_list_popfirst(dsk->operations,
				getReadyWriteOperations);
		if (op == NULL
		)
			break;

		if (op->client == NULL) {
			if (op->syncqueue == NULL) {
				log_clientlost(dsk->log, dsk->name, op->read, op->offset);
			} else
				collection_blist_push(op->syncqueue, NULL);
		} else {
			t_nipc *nipc = operation_getnipc(op);
			nipc_send(nipc, op->client);
			nipc_destroy(nipc);
		}
		operation_destroy(op);
	}

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

void disk_decreasepending(t_disk *dsk){
	sem_wait(&dsk->pendingsMutex);
	dsk->pendings++;
	sem_post(&dsk->pendingsMutex);
}

void disk_increasepending(t_disk *dsk){
	sem_wait(&dsk->pendingsMutex);
	dsk->pendings--;
	sem_post(&dsk->pendingsMutex);
}

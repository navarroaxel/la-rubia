#include "syncer.h"

void init_syncer(t_disk *dsk) {
	pthread_attr_t attr;
	pthread_t syncer_id;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&syncer_id, &attr, &syncer, dsk);
	pthread_attr_destroy(&attr);
}

void *syncer(void *args) {
	t_disk *dsk = args;
	dsk->offsetlimit = 0;
	t_blist *syncqueue = collection_blist_create(50);

	char thread_name[21];
	sprintf(thread_name, "syncer-%s", dsk->name);
	log_info(dsk->log, thread_name, "Se inicio la sincronizacion del disco %s",
			dsk->name);

	while (dsk->offsetlimit < raidoffsetlimit) {
		enqueueread(syncqueue, dsk->offsetlimit);

		t_disk_readSectorRs *rs = collection_blist_pop(syncqueue);
		enqueuewrite(syncqueue, dsk, rs);
		dsk->offsetlimit++;
		free(rs);

		t_disk_writeSectorRs *writeRs = collection_blist_pop(syncqueue);
		if (writeRs == NULL) {
			log_warning(dsk->log, thread_name,
					"Se interrumpio la sincronizacion");
			collection_blist_destroy(syncqueue);
			return NULL;
		}
		free(writeRs);

		log_info(dsk->log, thread_name,
				"Se sincronizo el disco %s hasta el sector %i", dsk->name,
				dsk->offsetlimit);
	}
	log_info(dsk->log, thread_name,
			"Se finalizo la sincronizacion del disco %s", dsk->name);
	collection_blist_destroy(syncqueue);
	return NULL;
}

void enqueueread(t_blist *syncqueue, uint32_t offset) {
	t_disk *dsk = disks_getidledisk(offset);
	dsk->pendings++;
	t_disk_readSectorRq *rq = malloc(sizeof(t_disk_readSectorRq));
	rq->offset = offset;

	t_nipc *nipc = nipc_create(NIPC_READSECTOR_RQ);
	nipc_setdata(nipc, rq, sizeof(t_disk_readSectorRq));

	t_operation *op = operation_create(nipc);
	op->disk = dsk->id;
	op->syncqueue = syncqueue;
	collection_list_add(dsk->operations, op);

	nipc_send(nipc, dsk->client);
	nipc_destroy(nipc);
}

void enqueuewrite(t_blist *syncqueue, t_disk *dsk, t_disk_readSectorRs *rs) {
	t_operation *op = malloc(sizeof(t_operation));
	t_disk_writeSectorRq *rq = malloc(sizeof(t_disk_writeSectorRq));
	op->offset = rq->offset = rs->offset;
	memcpy(rq->data, rs->data, DISK_SECTOR_SIZE);
	memcpy(op->data, rs->data, DISK_SECTOR_SIZE);

	dsk->pendings++;
	op->read = false;
	op->syncqueue = syncqueue;
	op->client = NULL;
	op->disk = dsk->id;
	collection_list_add(dsk->operations, op);

	t_nipc *nipc = nipc_create(NIPC_WRITESECTOR_RQ);
	nipc_setdata(nipc, rq, sizeof(t_disk_writeSectorRq));
	nipc_send(nipc, dsk->client);
	nipc_destroy(nipc);
}

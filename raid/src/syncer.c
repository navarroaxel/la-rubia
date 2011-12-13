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
	t_blist *reads = collection_blist_create(50);
	t_blist *writes = collection_blist_create(50);

	char thread_name[21];
	sprintf(thread_name, "syncer-%s", dsk->name);
	log_info(dsk->log, thread_name, "Se inicio la sincronizacion");
	int i;
	while (dsk->offsetlimit < raidoffsetlimit) {
		for (i = 0; i < 8 && dsk->offsetlimit + i < raidoffsetlimit; i++)
			enqueueread(reads, dsk->offsetlimit + i, thread_name);

		for (i = 0; i < 8 && dsk->offsetlimit + i < raidoffsetlimit; i++) {
			t_disk_readSectorRs *rs = collection_blist_pop(reads);
			enqueuewrite(writes, dsk, rs);
			free(rs);
		}
		for (i = 0; i < 8 && dsk->offsetlimit + i < raidoffsetlimit; i++) {
			t_disk_writeSectorRs *writeRs = collection_blist_pop(writes);
			if (writeRs == NULL) {
				log_warning(dsk->log, thread_name,
						"Se interrumpio la sincronizacion");
				collection_blist_destroy(reads);
				collection_blist_destroy(writes);
				return NULL;
			}
			free(writeRs);
		}
		dsk->offsetlimit += 8;
		log_info(dsk->log, thread_name,
				"Se sincronizaron los primeros %i sectores", dsk->offsetlimit);
	}
	log_info(dsk->log, thread_name,
			"Se finalizo la sincronizacion del disco %s", dsk->name);
	collection_blist_destroy(reads);
	collection_blist_destroy(writes);
	return NULL;
}

void enqueueread(t_blist *syncqueue, uint32_t offset, char *thread_name) {
	t_disk *dsk = disks_getidledisk(offset);
	disk_increasepending(dsk);
	t_disk_readSectorRq *rq = malloc(sizeof(t_disk_readSectorRq));
	rq->offset = offset;

	t_nipc *nipc = nipc_create(NIPC_READSECTOR_RQ);
	nipc_setdata(nipc, rq, sizeof(t_disk_readSectorRq));

	t_operation *op = operation_create(nipc);
	op->disk = dsk->id;
	op->syncqueue = syncqueue;
	collection_list_add(dsk->operations, op);

	log_info(dsk->log, thread_name, "LECTURA sector %i disco %s", op->offset,
			dsk->name);
	nipc_send(nipc, dsk->client);
	nipc_destroy(nipc);
}

void enqueuewrite(t_blist *syncqueue, t_disk *dsk, t_disk_readSectorRs *rs) {
	t_operation *op = malloc(sizeof(t_operation));
	t_disk_writeSectorRq *rq = malloc(sizeof(t_disk_writeSectorRq));
	op->offset = rq->offset = rs->offset;
	memcpy(rq->data, rs->data, DISK_SECTOR_SIZE);
	memcpy(op->data, rs->data, DISK_SECTOR_SIZE);

	disk_increasepending(dsk);
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

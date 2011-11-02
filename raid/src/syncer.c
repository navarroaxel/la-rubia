#include "syncer.h"

void init_syncer(t_disk *dsk) {
	pthread_attr_t attr;
	pthread_t syncer_id;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&syncer_id, &attr, &syncer, dsk);
	pthread_attr_destroy(&attr);
}

void *syncer(void *args){
	t_disk *dsk = args;
	t_blist *syncqueue = collection_blist_create(50);

	int i = 0;
	while (i < 20){
		enqueueread(i++, syncqueue);
	}

	while (dsk->offsetlimit < raidoffsetlimit) {
		t_disk_readSectorRs *rs = collection_blist_pop(syncqueue);
		enqueuewrite(dsk, rs);

		if (i < raidoffsetlimit)
			enqueueread(i, syncqueue);
	}
	return NULL;
}

void enqueueread(int offset, t_blist *syncqueue) {
	t_disk *dsk = disks_getidledisk();
	t_disk_readSectorRq *rq = malloc(sizeof(t_disk_readSectorRq));
	rq->offset = offset;

	t_nipc *nipc = nipc_create(NIPC_READSECTOR_RQ);
	nipc_setdata(nipc, rq, sizeof(t_disk_readSectorRq));

	t_operation *op = operation_create(nipc);
	op->syncqueue = syncqueue;
	collection_list_add(dsk->operations, op);

	nipc_send(nipc, dsk->client);
	nipc_destroy(nipc);
	dsk->pendings++;
}

void enqueuewrite(t_disk *dsk, t_disk_readSectorRs *rs){
	t_disk_writeSectorRq *rq = malloc(sizeof(t_disk_writeSectorRq));
	rq->offset = rs->offset;
	memcpy(rq->data, rs->data, DISK_SECTOR_SIZE);

	//TODO revisar si se encola este request en operations.

	t_nipc *nipc = nipc_create(NIPC_WRITESECTOR_RQ);
	nipc_setdata(nipc, rq, sizeof(t_disk_writeSectorRq));
	nipc_send(nipc, dsk->client);
	nipc_destroy(nipc);
	dsk->pendings++;
}

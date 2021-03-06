#include "dispatcher.h"

struct queue{
	t_blist *processed;
	t_log *log;
};

void init_dispatcher(t_blist *processed, t_log *log) {
	pthread_attr_t attr;
	pthread_t dispatcher_id;

	struct queue *q = malloc(sizeof(struct queue));
	q->processed = processed;
	q->log = log;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&dispatcher_id, &attr, &dispatcher, q);
	pthread_attr_destroy(&attr);
}

void *dispatcher(void *args) {
	struct queue *q = (struct queue *) args;
	t_disk_operation *e;
	t_nipc *nipc;
	while (true) {
		e = collection_blist_pop(q->processed);
		headtrace_log(e->headtrace, q->log);

 		if (e->client == NULL) {
			headtrace_destroy(e->headtrace);
			free(e);
			continue;
		}

		if (e->trace) {
			nipc= nipc_create(NIPC_DISKCONSOLE_TRACE);
			nipc_setdata(nipc, e->headtrace, sizeof(t_headtrace));
			nipc_send(nipc, e->client);
			nipc_destroy(nipc);
			free(e);
			continue;
		}

		nipc = nipc_create(e->read ? NIPC_READSECTOR_RS : NIPC_WRITESECTOR_RS);
		if (e->read) {
			t_disk_readSectorRs *rs = malloc(sizeof(t_disk_readSectorRs));
			memcpy(rs->data, e->data, DISK_SECTOR_SIZE);
			rs->offset = e->offset;
			rs->result = e->result;
			nipc_setdata(nipc, rs, sizeof(t_disk_readSectorRs));
		} else {
			t_disk_writeSectorRs *rs = malloc(sizeof(t_disk_writeSectorRs));
			rs->offset = e->offset;
			rs->result = e->result;
			nipc_setdata(nipc, rs, sizeof(t_disk_writeSectorRs));
		}

		nipc_send(nipc, e->client);
		nipc_destroy(nipc);
		headtrace_destroy(e->headtrace);
		free(e);
	}
	return NULL;
}

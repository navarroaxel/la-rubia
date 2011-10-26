#include "dispatcher.h"

void init_dispatcher(t_blist *ready) {
	pthread_attr_t attr;
	pthread_t dispatcher_id;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&dispatcher_id, &attr, &dispatcher, ready);
	pthread_attr_destroy(&attr);
}

void *dispatcher(void *args) {
	t_blist *processed = (t_blist *) args;
	t_disk_operation *e;
	while (true) {
		e = (t_disk_operation *) collection_blist_pop(processed);
		t_nipc *nipc = nipc_create(
				e->read ? NIPC_READSECTOR_RS : NIPC_WRITESECTOR_RS);

		if (e->client == NULL){
			free(e);
			continue;
		}

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

		t_socket_buffer *buffer = nipc_serializer(nipc);
		sockets_send(e->client, buffer->data, buffer->size);
		sockets_bufferDestroy(buffer);
		nipc_destroy(nipc);
	}
	return NULL;
}

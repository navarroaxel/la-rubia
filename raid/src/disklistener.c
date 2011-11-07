#include "disklistener.h"

struct t_queue {
	t_list *waiting;
	t_log *log;
};

void init_disklistener(t_list *waiting, t_log *log) {
	pthread_attr_t attr;
	pthread_t console_id;

	struct t_queue *q = malloc (sizeof(struct t_queue));
	q->waiting = waiting;
	q->log = log;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&console_id, &attr, &disklistener, (void *)q);
	pthread_attr_destroy(&attr);
}

void *disklistener(void *args){
	struct t_queue *q = (struct t_queue *)args;
	t_socket_server *server = sockets_createServer(NULL, config->diskPort);
	if (server == NULL){
		log_error(q->log, "DISKLISTENER", "Socket Server Disk es NULL");
		exit(EXIT_FAILURE);
	}

	if (!sockets_listen(server)){
		log_error(q->log, "DISKLISTENER", "Socket Server Disk no puede escuchar");
		exit(EXIT_FAILURE);
	}

	while (true) {
		t_socket_client *client= sockets_accept(server);
		t_socket_buffer *buffer = sockets_recv(client);
		if (buffer == NULL){
			sockets_destroyClient(client);
			continue;
		}

		t_nipc *nipc = nipc_deserializer(buffer, 0);
		sockets_bufferDestroy(buffer);
		if (!disklistener_handshake(client, nipc, q->waiting, q->log))
			sockets_destroyClient(client);

		nipc_destroy(nipc);
	}
	return NULL;
}

int disklistener_handshake(t_socket_client *client, t_nipc *rq, t_list *waiting, t_log *log){
	t_disk *dsk;
	t_nipc *nipc;
	char diskname[13];

	if (rq->type != NIPC_HANDSHAKE || rq->length == 0){
		log_warning(log, "DISKLISTENER", "Handshake invalido");
		return false;
	}

	if (rq->length > 13){
		log_warning(log, "DISKLISTENER", "Handshake invalido. Nombre de disco mayor a 13 chars");
		return false;
	}

	memcpy(diskname, rq->payload, rq->length);

	dsk = disks_getbyname(diskname);
	if (dsk != NULL){
		log_warning(log, "DISKLISTENER", "ERROR CONEXION disco con nombre repetido: %s", diskname);
		nipc = nipc_create(NIPC_ERROR);
		nipc_setdata(nipc, strdup("Nombre repetido"), strlen("Nombre repetido")+1);
		nipc_send(nipc, client);
		nipc_destroy(nipc);
		return false;
	}

	nipc = nipc_create(NIPC_HANDSHAKE);
	nipc_setdata(nipc, NULL, 0);
	nipc_send(nipc, client);
	nipc_destroy(nipc);

	t_socket_buffer *buffer = sockets_recv(client);
	nipc = nipc_deserializer(buffer, 0);
	sockets_bufferDestroy(buffer);
	t_disk_chs *chs = nipc->payload;
	if (nipc->type != NIPC_DISKCHS){
		nipc_destroy(nipc);
		return false;
	}

	bool syncdisk = false;
	if (raidoffsetlimit != 0) {
		syncdisk = true;
		if (chs->cylinders * chs->heads * chs->sectors < raidoffsetlimit) {
			log_warning(log, "DISKLISTENER", "ERROR CONEXION disco con CHS invalido: %s (%i,%i,%i)", diskname, chs->cylinders, chs->heads, chs->sectors);
			nipc_destroy(nipc);
			nipc = nipc_create(NIPC_ERROR);
			nipc_setdata(nipc, strdup("Invalid CHS"), strlen("Invalid CHS"));
			nipc_send(nipc, client);
			nipc_destroy(nipc);
			return false;
		}
	} else
		raidoffsetlimit = chs->cylinders * chs->heads * chs->sectors;

	nipc_destroy(nipc);
	nipc = nipc_create(NIPC_DISKCHS);
	nipc_setdata(nipc, NULL, 0);
	nipc_send(nipc, client);
	nipc_destroy(nipc);

	log_info(log, "DISKLISTENER", "Se ha conectado el disco %s", diskname);
	dsk = disks_register(diskname, client, waiting, log);
	if (syncdisk)
		init_syncer(dsk);
	else
		dsk->offsetlimit = raidoffsetlimit;

	return true;
}

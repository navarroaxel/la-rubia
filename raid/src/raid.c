#include "raid.h"
#include "common/utils/config.h"
void diskconnect(void);

config_raid *config;
t_list *disks;
uint32_t raidoffsetlimit;
void diskconnect(void);

int main(void) {
	t_xmlFile *configFile = loadConfig("config.xml");
	config = xmlGetConfigStructRaid(configFile);

	disks_init();

	t_list *waiting = collection_list_create();
	t_log *log = log_create("RAID", config->logFilePath, WARNING | DEBUG | ERROR | INFO, config->consoleEnabled ? M_CONSOLE_ENABLE : M_CONSOLE_DISABLE);

	init_disklistener(waiting, log);
	listener(waiting, log);

	return EXIT_SUCCESS;
}

void diskconnect(void) {
	t_socket_client *client = sockets_createClient(NULL, 5500);

	sockets_connect(client, "127.0.0.1", 5000);
	t_nipc *nipc = nipc_create(NIPC_HANDSHAKE);
	nipc_setdata(nipc, NULL, 0);

	nipc_send(nipc, client);
	t_socket_buffer *buffer = sockets_recv(client);
	nipc = nipc_deserializer(buffer, 0);

	t_disk_readSectorRq *rq = malloc(sizeof(t_disk_readSectorRq));
	rq->offset = 0;

	nipc = nipc_create(NIPC_READSECTOR_RQ);
	t_disk_readSectorRs *rs;
	t_nipc *nipc2;
	nipc_setdata(nipc, rq, sizeof(t_disk_readSectorRq));
	nipc_send(nipc, client);

	buffer = sockets_recv(client);
	nipc2 = nipc_deserializer(buffer, 0);
	rs = nipc2->payload;
	exit(0);
}

void listener(t_list *waiting, t_log *log) {
	t_socket_server *server = sockets_createServer(NULL, config->fsPort);
	if (server == NULL){
		log_error(log, "FSLISTENER", "Socket Server File System es NULL");
		return;
	}

	if (!sockets_listen(server)){
		log_error(log, "FSLISTENER", "Socket Server File System no puede escuchar");
		return;
	}

	t_list *servers = collection_list_create();
	collection_list_add(servers, server);

	t_socket_client *acceptClosure(t_socket_server *server){
		t_socket_client *client= sockets_accept(server);
		t_socket_buffer *buffer = sockets_recv(client);
		if (buffer == NULL){
			sockets_destroyClient(client);
			return NULL;
		}

		t_nipc *nipc = nipc_deserializer(buffer, 0);
		sockets_bufferDestroy(buffer);
		if (!handshake(client, nipc, waiting, log)){
			nipc_destroy(nipc);
			sockets_destroyClient(client);
			return NULL;
		}
		nipc_destroy(nipc);
		return client;
	}

	int recvClosure(t_socket_client * client) {
		t_socket_buffer *buffer = sockets_recv(client);
		if (buffer == NULL)
			return false;

		t_nipc *nipc = nipc_deserializer(buffer, 0);
		enqueueoperation(nipc, client, waiting, log);

		nipc_destroy(nipc);
		sockets_bufferDestroy(buffer);

		return true;
	}

	t_list *clients = collection_list_create();
	while (true) {
		sockets_select(servers, clients, 0, &acceptClosure, &recvClosure);
	}
}

int handshake(t_socket_client *client, t_nipc *rq, t_list *waiting, t_log *log) {
	if (rq->type != NIPC_HANDSHAKE || rq->length != 0) {
		log_warning(log, "FSLISTENER", "Handshake invalido");
		return false;
	}

	if (disks_size() == 0) {
		log_warning(log, "FSLISTENER", "Conexion rechazada, no hay discos conectados");
		t_nipc *nipc = nipc_create(NIPC_HANDSHAKE);
		nipc_setdata(nipc, strdup("No hay discos conectados."), strlen("No hay discos conectados.") + 1);

		nipc_send(nipc, client);
		nipc_destroy(nipc);
		return false;
	}

	t_nipc *nipc = nipc_create(NIPC_HANDSHAKE);
	nipc_setdata(nipc, NULL, 0);
	nipc_send(nipc, client);
	nipc_destroy(nipc);
	return true;
}

void enqueueoperation(t_nipc *nipc, t_socket_client *client, t_list *waiting, t_log *log) {
	t_operation *op = operation_create(nipc);
	if (op == NULL){
		log_warning(log, "FSLISTENER", "Llego un pedido invalido");
		return;
	}
	op->client = client;
	collection_list_add(waiting, op);
	if (op->read) {
		t_disk *dsk = disks_getidledisk(op->offset);
		dsk->pendings++;
		op->disk = dsk->id;
		log_info(log, "FSLISTENER", "LECTURA sector %i disco %s", op->offset, dsk->name);
		nipc_send(nipc, dsk->client);
	} else {
		t_socket_buffer *buffer = nipc_serializer(nipc);
		void sendrequest(void *data) {
			struct t_disk *disk = (struct t_disk *) data;
			sockets_sendBuffer(disk->client, buffer);
			op->disk |= disk->id;
			disk->pendings++;
		}
		log_info(log, "FSLISTENER", "ESCRITURA sector %i todos los discos", op->offset);
		collection_list_iterator(disks, sendrequest);
	}
}

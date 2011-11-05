#include "listener.h"

extern config_disk * config;

void log_incomingrequest(t_log *logFile, bool read, uint32_t offset) {
	log_info(logFile, "LISTENER", "LLEGO PEDIDO\nTipo: %s\nSector: %i",
			read ? "lectura" : "escritura", offset);
}

void listener(t_blist *waiting, t_log *logFile) {
	t_socket_server *server = sockets_createServer("127.0.0.1",
			config->bindPort);

	if (server == NULL) {
		log_error(logFile, "LISTENER", "Socket Server es NULL");
		return;
	}

	sockets_listen(server);

	t_list *servers = collection_list_create();
	collection_list_add(servers, server);

	int recvClient(t_socket_client * client) {
		t_socket_buffer *buffer = sockets_recv(client);
		uint32_t offsetInBuffer = 0;
		t_nipc *nipc;
		if (buffer == NULL
		)
			return false;

		while (offsetInBuffer < buffer->size) {
			nipc = nipc_deserializer(buffer, offsetInBuffer);
			offsetInBuffer += nipc->length + sizeof(nipc->type)
					+ sizeof(nipc->length);

			if (nipc->type == NIPC_HANDSHAKE) {
				if (handshakeNewClient(client, nipc) == 0)
					return client->socket->desc;

				return false;
			}
			t_disk_operation *op = getdiskoperation(nipc, client);
			nipc_destroy(nipc);
			if (op == NULL
			)
				return false;

			log_incomingrequest(logFile, op->read, op->offset);

			enqueueOperation(waiting, op);
		}
		sockets_bufferDestroy(buffer);
		return client->socket->desc;
	}

	t_list *clients = collection_list_create();

	while (true) {
		sockets_select(servers, clients, 0, NULL, &recvClient);
	}
}

void connectraid(t_blist *waiting, t_log *logFile) {
	t_socket_client *client = sockets_createClient(config->bindIp,
			config->bindPort + 1);

	sockets_connect(client, config->raidIp, config->raidPort);
	if (!handshake(client, logFile))
		return;

	t_socket_buffer *buffer;
	t_nipc *nipc;
	int offsetInBuffer;
	t_disk_operation *op;
	while (true) {
		offsetInBuffer = 0;
		buffer = sockets_recv(client);
		if (buffer == NULL) {
			log_error(logFile, "LISTENER", "RAID Desconectado");
			return;
		}

		while (offsetInBuffer < buffer->size) {
			nipc = nipc_deserializer(buffer, offsetInBuffer);
			offsetInBuffer += nipc->length + sizeof(nipc->type)
					+ sizeof(nipc->length);

			op = getdiskoperation(nipc, client);
			nipc_destroy(nipc);
			if (op == NULL
				)
				continue;

			enqueueOperation(waiting, op);

			log_incomingrequest(logFile, op->read, op->offset);
		}
		sockets_bufferDestroy(buffer);
	}
	sockets_destroyClient(client);
}

int handshake(t_socket_client *client, t_log *logFile) {
	t_nipc *nipc = nipc_create(NIPC_HANDSHAKE);

	nipc_setdata(nipc, strdup(config->diskname), strlen(config->diskname) + 1);

	nipc_send(nipc, client);
	nipc_destroy(nipc);

	t_socket_buffer *buffer = sockets_recv(client);
	nipc = nipc_deserializer(buffer, 0);
	sockets_bufferDestroy(buffer);
	if (nipc->type != NIPC_HANDSHAKE || nipc->length != 0) {
		log_error(logFile, "LISTENER", "Error al conectar con el RAID");
		nipc_destroy(nipc);
		return false;
	}

	nipc_destroy(nipc);

	nipc = nipc_create(NIPC_DISKCHS);
	t_disk_chs *chs = malloc(sizeof(t_disk_chs));
	chs->cylinders = config->cylinders;
	chs->heads = config->heads;
	chs->sectors = config->sectors;

	nipc_setdata(nipc, chs, sizeof(t_disk_chs));

	nipc_send(nipc, client);
	nipc_destroy(nipc);

	buffer = sockets_recv(client);
	nipc = nipc_deserializer(buffer, 0);
	sockets_bufferDestroy(buffer);

	if (nipc->type == NIPC_DISKCHS && nipc->length == 0) {
		nipc_destroy(nipc);
		return true;
	} else {
		log_error(logFile, "LISTENER", "Error al conectar con el RAID: %s",
				nipc->length > 0 ? nipc->payload : "");
		nipc_destroy(nipc);
		return false;
	}
}

int handshakeNewClient(t_socket_client *client, t_nipc *nipc2) {
	t_nipc *nipc;
	if (strcmp(config->mode, "LISTEN") != 0) {
		nipc = nipc_create(NIPC_ERROR);
		nipc_setdata(nipc, strdup("Modo invalido"),
				strlen("Modo invalido") + 1);
		nipc_send(nipc, client);
		nipc_getdata_destroy(nipc);
		return false;
	}
	nipc = nipc_create(NIPC_HANDSHAKE);
	nipc_setdata(nipc, NULL, 0);
	nipc_send(nipc, client);
	nipc_destroy(nipc);
	return true;
}

t_disk_operation *getdiskoperation(t_nipc *nipc, t_socket_client *client) {
	t_disk_operation *operation = malloc(sizeof(t_disk_operation));
	operation->trace = false;
	switch (nipc->type) {
	case NIPC_READSECTOR_RQ:
		operation->read = true;
		operation->offset = ((t_disk_readSectorRq *) nipc->payload)->offset;
		break;
	case NIPC_WRITESECTOR_RQ:
		operation->read = false;
		t_disk_writeSectorRq *rq = (t_disk_writeSectorRq *) nipc->payload;
		operation->offset = rq->offset;
		memcpy(&operation->data, rq->data, DISK_SECTOR_SIZE);
		break;
	default:
		return NULL;
	}
	operation->client = client;
	return operation;
}


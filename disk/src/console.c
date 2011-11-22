#include "console.h"

extern config_disk *config;
void init_console(t_blist *waiting) {
	pthread_attr_t attr;
	pthread_t console_id;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&console_id, &attr, &console, (void*) waiting);
	pthread_attr_destroy(&attr);
}

void *console(void *args) {
	t_blist *waiting = args;
	t_socket_server *server = sockets_createServerUnix(config->socketunixpath);
	if (server == NULL) {
		perror("No se pudo crear el socket server de la consola");
		return NULL;
	}
	if (!sockets_listen(server)) {
		perror("Socket Server cannot listen");
		return NULL;
	}

	if (fork() == 0) {
		char* arg_list[] = { "console", config->socketunixpath, NULL };
		execvp(config->consolePath, arg_list);
		perror("Error en execvp\n");
		abort();
	}

	t_socket_client *client = sockets_acceptUnix(server);
	if (client == NULL) {
		perror("No se pudo establecer conexion con el cliente de la consola");
		return NULL;
	}
	t_nipc *nipc;
	t_socket_buffer *buffer;
	while (true) {
		buffer = sockets_recv(client);
		if (buffer == NULL) {
			perror("la consola se ha desconectado");
			sockets_destroyClient(client);
			sockets_destroyServer(server);
			return NULL;
		}
		nipc = nipc_deserializer(buffer, 0);
		switch (nipc->type) {
		case NIPC_DISKCONSOLE_INFO:
			sockets_bufferDestroy(buffer);
			nipc_destroy(nipc);
			nipc = nipc_create(NIPC_DISKCONSOLE_INFO);
			nipc_setdata(nipc, head_currentlocation(), sizeof(t_location));
			nipc_send(nipc, client);
			nipc_destroy(nipc);

			break;
		case NIPC_DISKCONSOLE_CLEAN: {
			int offset = sizeof(char);
			int tmpsize;
			uint32_t sector, sectorto;

			memcpy(&sector, buffer->data + offset, tmpsize = sizeof(uint32_t));
			offset += tmpsize;
			memcpy(&sectorto, buffer->data + offset, tmpsize);

			t_disk_operation *op;
			for (; sector <= sectorto; sector++) {
				op = malloc(sizeof(t_disk_operation));
				op->client = NULL;
				op->offset = sector;
				op->read = 0;
				memset(op->data, 0, sizeof(op->data));
				enqueueOperation(waiting, op);
			}
		}
			break;
		case NIPC_DISKCONSOLE_TRACE: {
			int offset = sizeof(char);
			int tmpsize = sizeof(uint32_t);
			while (offset < buffer->size) {
				t_disk_operation *op = malloc(sizeof(t_disk_operation));
				memcpy(&op->offset, buffer->data + offset, tmpsize);
				op->trace = true;
				op->client = client;
				op->read = true;
				enqueueOperation(waiting, op);
				offset += tmpsize;
			}
		}
			break;
		}

		sockets_bufferDestroy(buffer);
	}
	return NULL;
}

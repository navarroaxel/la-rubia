#include "pconsole.h"
#include <pthread.h>

extern config_disk *config;
void init_pconsole(void){
	pthread_attr_t attr;
	pthread_t console_id;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&console_id, &attr, &pconsole, NULL);
	pthread_attr_destroy(&attr);
}

void *pconsole(void* args) {
	printf("Iniciando...\n");
	sleep(5);

	t_socket_client *client = sockets_createClientUnix(config->socketunixpath);
	sockets_connectUnix(client, SOCKET_UNIX_PATH);

	t_commands *cmd = commands_createWithConext(client, ' ', '\n', ' ');

	commands_add(cmd, "info", info);
	commands_add(cmd, "clean", clean);
	commands_add(cmd, "trace", trace);

	char input[100];
	while (true) {
		printf("> ");
		fflush(stdin);
		fgets(input, sizeof(input), stdin);
		commands_parser(cmd, input);
	}

	commands_destroy(cmd);
	return NULL;
}

void info(void *context, t_array *args) {
	t_socket_client *client = context;
	char code = 1;
	sockets_send(client, &code, sizeof(char));

	t_socket_buffer *buffer = sockets_recv(client);

	if (buffer->data[0] == 1) {
		t_location *location = location_create(0);
		int offset = 1;
		int tmpsize;
		memcpy(&location->cylinder, buffer->data + offset, tmpsize = sizeof(uint16_t));
		offset += tmpsize;
		memcpy(&location->sector, buffer->data+offset, tmpsize);

		printf("%i:%i\n", location->cylinder, location->sector);
	}

	sockets_bufferDestroy(buffer);
}

void clean(void *context, t_array *args) {
	if (args == NULL || array_size(args) != 2) {
		printf("clean <sector inicial> <sector final>\n");
		return;
	}
}

void trace(void *context, t_array *args) {
	if (args == NULL || array_size(args) > 5) {
		printf("trace [lista sectores]\n");
		return;
	}
}

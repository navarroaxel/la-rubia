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
	char code = CONSOLE_INFO;
	sockets_send(client, &code, sizeof(char));

	t_socket_buffer *buffer = sockets_recv(client);

	if (buffer->data[0] == CONSOLE_INFO) {
		t_location *location = malloc(sizeof(t_location));
		memcpy(location, buffer->data + sizeof(char), sizeof(t_location));
		printf("%i:%i\n", location->cylinder, location->sector);
	}

	sockets_bufferDestroy(buffer);
}

void clean(void *context, t_array *args) {
	if (args == NULL || array_size(args) != 2) {
		printf("clean <sector inicial> <sector final>\n");
		return;
	}

	int offset, tmpsize;
	t_socket_client *client = context;
	t_socket_buffer *buffer = malloc(sizeof(t_socket_buffer));

	uint32_t value = CONSOLE_CLEAN;

	memcpy(buffer->data, &value, offset = sizeof(char));
	value = atol(array_get(args, 0));
	memcpy(buffer->data + offset, &value, tmpsize = sizeof(uint32_t) );
	value = atol(array_get(args, 1));
	offset += tmpsize;
	memcpy(buffer->data + offset, &value, tmpsize);
	buffer->size = offset + tmpsize;

	sockets_sendBuffer(client, buffer);

	sockets_bufferDestroy(buffer);
}

void trace(void *context, t_array *args) {
	if (args == NULL || array_size(args) > 5) {
		printf("trace [lista sectores]\n");
		return;
	}

	int offset, tmpsize, i;
	t_socket_client *client = context;
	t_socket_buffer *buffer = malloc(sizeof(t_socket_buffer));

	uint32_t value = CONSOLE_TRACE;
	memcpy(buffer->data, &value, offset = sizeof(char));
	tmpsize = sizeof(uint32_t);
	for(i = 0; i < array_size(args); i++) {
		value = atol(array_get(args, i));
		memcpy(buffer->data + offset, &value, tmpsize);
		offset += tmpsize;
	}
	buffer->size = offset;
	sockets_sendBuffer(client, buffer);
	sockets_bufferDestroy(buffer);

	for (i = 0; i < array_size(args); i++){
		buffer = sockets_recv(client);
		t_headtrace *trace = headtrace_create();
		memcpy(trace, buffer->data, buffer->size);
		headtrace_printf(trace);
		headtrace_destroy(trace);
		sockets_bufferDestroy(buffer);
	}
}

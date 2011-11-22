#include "console.h"

int main(int argc, char * const argv[]) {
	if (argc != 2) {
		perror("Invalid argument count");
		return EXIT_FAILURE;
	}

	t_socket_client *client = sockets_createClientUnix(argv[1]);
	if (client == NULL){
		perror("Error al crear el socket cliente");
		return EXIT_FAILURE;
	}
	if (!sockets_connectUnix(client, argv[1])){
		perror("Error al conectarse al proceso disco");
		return EXIT_FAILURE;
	}

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
	return EXIT_SUCCESS;
}

void info(void *context, t_array *args) {
	t_socket_client *client = context;
	t_nipc *nipc = nipc_create(NIPC_DISKCONSOLE_INFO);
	nipc_send(nipc,client);
	nipc_destroy(nipc);

	t_socket_buffer *buffer = sockets_recv(client);
	nipc = nipc_deserializer(buffer);

	if (nipc->type == NIPC_DISKCONSOLE_INFO) {
		t_location *location = nipc_getdata(nipc);
		printf("%i:%i\n", location->cylinder, location->sector);
	}

	nipc_destroy(nipc);
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

	uint32_t value = NIPC_DISKCONSOLE_CLEAN;

	memcpy(buffer->data, &value, offset = sizeof(char));
	value = atol(array_get(args, 0));
	memcpy(buffer->data + offset, &value, tmpsize = sizeof(uint32_t));
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

	uint32_t value = NIPC_DISKCONSOLE_TRACE;
	memcpy(buffer->data, &value, offset = sizeof(char));
	tmpsize = sizeof(uint32_t);
	for (i = 0; i < array_size(args); i++) {
		value = atol(array_get(args, i));
		memcpy(buffer->data + offset, &value, tmpsize);
		offset += tmpsize;
	}
	buffer->size = offset;
	sockets_sendBuffer(client, buffer);
	sockets_bufferDestroy(buffer);

	for (i = 0; i < array_size(args);) {
		buffer = sockets_recv2(client);
		offset = 0;
		do {
			headtrace_printf((t_headtrace *) (buffer->data + offset));
			offset += sizeof(t_headtrace);
			i++;
		} while (offset < buffer->size);
		sockets_bufferDestroy(buffer);
	}
}

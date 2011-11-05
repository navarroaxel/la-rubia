#include "console.h"

int main(int argc, char * const argv[]) {
	if (argc != 2) {
		perror("Invalid argument count");
		return EXIT_FAILURE;
	}
	t_socket_client *client = sockets_createClientUnix(argv[1]);
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
	return EXIT_SUCCESS;
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

	uint32_t value = CONSOLE_TRACE;
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
		buffer = sockets_recv(client);
		offset = 0;
		do {
			headtrace_printf((t_headtrace *) (buffer->data + offset));
			offset += sizeof(t_headtrace);
			i++;
		} while (offset < buffer->size);
		sockets_bufferDestroy(buffer);
	}
}

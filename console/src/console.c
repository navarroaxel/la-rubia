#include "console.h"

int main(int argc, char *const argv[]) {
	printf(argv[0]);
	t_socket_client *client = sockets_createClientUnix(argv[0]);
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
	char code = 0;//CONSOLE_INFO;
	sockets_send(client, &code, sizeof(char));

	t_socket_buffer *buffer = sockets_recv(client);

	if (buffer->data[0] == 0){//CONSOLE_INFO) {
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

	int offset, tmpsize;
	t_socket_client *client = context;
	t_socket_buffer *buffer = malloc(sizeof(t_socket_buffer));

	uint32_t value = 1;//CONSOLE_CLEAN;

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

	uint32_t value = 2;//CONSOLE_TRACE;
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

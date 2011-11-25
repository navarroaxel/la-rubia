#include "console.h"

int main(int argc, char * const argv[]) {
	if (argc != 2) {
		perror("Invalid argument count");
		return EXIT_FAILURE;
	}

	t_socket_client *client = sockets_createClientUnix(argv[1]);
	if (client == NULL) {
		perror("Error al crear el socket cliente");
		return EXIT_FAILURE;
	}
	if (!sockets_connectUnix(client, argv[1])) {
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
	nipc_send(nipc, client);
	nipc_destroy(nipc);

	t_socket_buffer *buffer = sockets_recv(client);
	nipc = nipc_deserializer(buffer, 0);

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

	uint32_t *sectors =malloc(sizeof(uint32_t) * 2);
	t_socket_client *client = context;
	t_nipc *nipc = nipc_create(NIPC_DISKCONSOLE_CLEAN);

	sectors[0] = atol(array_get(args, 0));
	sectors[1] = atol(array_get(args, 1));
	nipc_setdata(nipc, sectors, 2 * sizeof(uint32_t));
	nipc_send(nipc, client);
	nipc_destroy(nipc);
}

void trace(void *context, t_array *args) {
	if (args == NULL || array_size(args) > 5) {
		printf("trace [lista sectores]\n");
		return;
	}

	int i;
	uint32_t *sectors =malloc(sizeof(uint32_t) * 5);
	t_socket_client *client = context;
	t_nipc *nipc = nipc_create(NIPC_DISKCONSOLE_TRACE);
	for (i = 0; i < array_size(args); i++) {
		sectors[i] = atol(array_get(args, i));
	}
	nipc_setdata(nipc, sectors, i * sizeof(uint32_t));
	nipc_send(nipc, client);
	nipc_destroy(nipc);

	for (i = 0; i < array_size(args);) {
		t_socket_buffer *buffer = sockets_recv(client);
		nipc = nipc_deserializer(buffer, 0);
		sockets_bufferDestroy(buffer);

		headtrace_printf((t_headtrace *) nipc_getdata(nipc));

		nipc_destroy(nipc);
	}
}

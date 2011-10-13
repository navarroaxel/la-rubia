#include "pconsole.h"

t_socket_client *client;

void pconsole(void) {
	printf("Iniciando...\n");
	sleep(3);
	t_commands *cmd = commands_create(' ', '\n', ' ');

	commands_add(cmd, "info", info);
	commands_add(cmd, "clean", clean);
	commands_add(cmd, "trace", trace);

	client = sockets_createClientUnix(SOCKET_UNIX_PATH);
	sockets_connectUnix(client, SOCKET_UNIX_PATH);

	char input[100];
	while (true) {
		printf("> ");
		fflush(stdin);
		fgets(input, sizeof(input), stdin);
		commands_parser(cmd, input);
	}

	commands_destroy(cmd);
}

void info(void *context, t_array *args) {
	if (args != NULL) {
		printf("info\n");
		return;
	}
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

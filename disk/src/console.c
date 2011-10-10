#include "console.h"

void console(void) {
	t_commands *cmd =commands_create(' ','\n', ' ');

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

	/*sleep(5);
	 t_socket_client *sckt_un = sockets_createClientUnix(SOCKET_UNIX_PATH);
	 if (sockets_connectUnix(sckt_un, SOCKET_UNIX_PATH)) {
	 sockets_write(sckt_un, "ASD", 4);
	 printf("success\n");
	 } else {
	 printf("%s\n", strerror(errno));
	 printf("%i\n", errno);
	 printf("fallo\n");
	 }*/
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

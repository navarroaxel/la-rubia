#include "disk.h"
#include "fcntl.h"
#include "listener.h"
#include "head.h"
#include "dispatcher.h"
#include <errno.h>
#include <stdbool.h>

int main(void) {
	/*
	 if (fork() == 0) {
	 console();
	 return EXIT_SUCCESS;
	 }*/

	t_blist *waiting = collection_blist_create(50);
	t_blist *processed = collection_blist_create(50);

	init_head(waiting, processed);

	init_dispatcher(processed);

	listener(waiting);

	return EXIT_SUCCESS;
}

void console(void) {
	char input[100];
	while (true) {
		printf("> ");
		fflush(stdin);
		fgets(input, sizeof(input), stdin);
		printf("%s\n", input);
		if (!strncmp("info", input, strlen("info"))) {
			info();
		} else if (!strncmp("clean", input, strlen("clean"))) {
			clean(input + strlen("clean"));
		} else if (!strncmp("trace", input, strlen("trace"))) {
			trace(input + strlen("trace"));
		}
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

void info(void) {

}

void clean(char *input) {

}

void trace(char *input) {
	int i;
	char *subs[10];
	for (i = 0; i < 10; i++) {
		subs[i] = malloc(10);
	}

	string_split3(input, ' ', subs);

	i = 0;
	while (subs[i] != NULL) {
		printf("%s\n", subs[i++]);
	}
}


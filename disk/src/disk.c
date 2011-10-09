#include "disk.h"
#include "fcntl.h"
#include "listener.h"
#include "head.h"
#include "dispatcher.h"
#include <errno.h>
#include <stdbool.h>

int main(void) {
	if (fork() == 0) {
		console();
		return EXIT_SUCCESS;
	}

	t_blist *waiting = collection_blist_create(50);
	t_blist *processed = collection_blist_create(50);

	init_head(waiting, processed);

	init_dispatcher(processed);

	listener(waiting);

	return EXIT_SUCCESS;
}

void console(void) {
	sleep(5);
	char input[255];
	while (true) {
		printf("> ");
		fgets(input, 255, stdin);
		printf("%s\n", input);
		if (strcmp("info", input)) {
			printf("info command\n");
		}
	}

	/*t_socket_client *sckt_un = sockets_createClientUnix(SOCKET_UNIX_PATH);
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

void clean(void) {

}

void trace(void) {

}

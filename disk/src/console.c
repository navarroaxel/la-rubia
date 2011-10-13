#include "console.h"

void init_console(void) {
	pthread_attr_t attr;
	pthread_t console_id;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&console_id, &attr, &console, NULL);
	pthread_attr_destroy(&attr);
}

void *console(void *args) {
	t_socket_server *server = sockets_createServerUnix(SOCKET_UNIX_PATH);

	 sockets_listen(server);
	 t_socket_client *a = sockets_acceptUnix(server);

	 t_socket_buffer *buffer;
	 while(true){
		 buffer = sockets_recv(a);

		 //TODO process this msg!!

		 sockets_bufferDestroy(buffer);
	 }
	 return NULL;
}

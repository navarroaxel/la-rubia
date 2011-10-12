#include "console.h"

void console(void){
	t_socket_server *server = sockets_createServerUnix(SOCKET_UNIX_PATH);

	 sockets_listen(server);
	 t_socket_client *a = sockets_acceptUnix(server);

	 t_socket_buffer *buffer;
	 while(true){
		 buffer = sockets_recv(a);

		 //TODO process this msg!!

		 sockets_bufferDestroy(buffer);
	 }
}

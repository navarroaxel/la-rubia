#include "console.h"

extern config_disk *config;
void init_console(void) {
	pthread_attr_t attr;
	pthread_t console_id;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&console_id, &attr, &console, NULL);
	pthread_attr_destroy(&attr);
}

void *console(void *args) {
	t_socket_server *server = sockets_createServerUnix(config->socketunixpath);

	 sockets_listen(server);
	 t_socket_client *client = sockets_acceptUnix(server);

	 t_socket_buffer *buffer;
	 while(true){
		 buffer = sockets_recv(client);

		 switch(buffer->data[0])
		 {
		 case 1:
			 sockets_bufferDestroy(buffer);
			 int tmpsize;
			 int offset = sizeof(char);
			 buffer = malloc(sizeof(t_socket_buffer));
			 t_location *location = head_currentlocation();

			 buffer->data[0] = 1;
			 memcpy(buffer->data + offset, &location->cylinder, tmpsize = sizeof(uint16_t));
			 offset += tmpsize;
			 memcpy(buffer->data + offset, &location->sector, tmpsize);
			 buffer->size = offset + tmpsize;

			 sockets_sendBuffer(client, buffer);
			 break;
		 }

		 sockets_bufferDestroy(buffer);
	 }
	 return NULL;
}

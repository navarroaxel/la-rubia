#ifndef LISTENER_H_
#define LISTENER_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "common/utils/sockets.h"
#include "common/collections/blist.h"
#include "common/nipc.h"
#include "disk.h"

#define SOCKET_UNIX_PATH "/tmp/lol"

void enqueueOperation(t_blist *waiting, t_disk_operation *op);
void listener(t_blist *list);
t_disk_operation *getdiskoperation(t_nipc *nipc, t_socket_client *client);
int handshake(t_socket_client *client, t_nipc *nipc2);

#endif

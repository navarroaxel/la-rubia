#ifndef LISTENER_H_
#define LISTENER_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "common/utils/sockets.h"
#include "common/utils/config.h"
#include "common/collections/blist.h"
#include "common/nipc.h"
#include "common/utils/log.h"
#include "disk.h"
#include "headHelper.h"

void enqueueOperation(t_blist *waiting, t_disk_operation *op);
void listener(t_blist *waiting, t_log *logFile);
void connectraid(t_blist *waiting, t_log *logFile);
t_disk_operation *getdiskoperation(t_nipc *nipc, t_socket_client *client);
int handshake(t_socket_client *client, t_log *logFile);
int handshakeNewClient(t_socket_client *client, t_nipc *rq, t_log *logFile);

#endif

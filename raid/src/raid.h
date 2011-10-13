#ifndef RAID_H_
#define RAID_H_

#include <stdlib.h>

#include <stdbool.h>
#include <pthread.h>
#include <stdint.h>
#include "common/nipc.h"
#include "common/utils/sockets.h"
#include "common/collections/list.h"
#include "operations.h"
#include "disk.h"

t_list *disks;
void listener(t_list *waiting);
int handshake(t_socket_client *client, t_nipc *rq, t_list *waiting);
void enqueueoperation(t_nipc *nipc, t_socket_client *client, t_list *wainting);

#endif

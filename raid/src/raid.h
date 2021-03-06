#ifndef RAID_H_
#define RAID_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "common/nipc.h"
#include "common/utils/sockets.h"
#include "common/utils/config.h"
#include "common/collections/list.h"
#include "disklistener.h"
#include "syncer.h"
#include "operations.h"
#include "disk.h"

extern uint32_t raidoffsetlimit;

void listener(t_list *waiting, t_log *log);
int handshake(t_socket_client *client, t_nipc *rq, t_list *waiting, t_log *log);
void enqueueoperation(t_nipc *nipc, t_socket_client *client, t_list *waiting, t_log *log);

#endif

#ifndef DISKLISTENER_H_
#define DISKLISTENER_H_

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "common/nipc.h"
#include "common/utils/sockets.h"
#include "common/utils/config.h"
#include "common/collections/list.h"
#include "syncer.h"
#include "disk.h"

extern uint32_t raidoffsetlimit;
extern config_raid *config;

void init_disklistener(t_list *waiting, t_log *log);
void *disklistener(void *args);
int disklistener_handshake(t_socket_client *client, t_nipc *rq, t_list *waiting, t_log *log);

#endif

#ifndef DISKS_H_
#define DISKS_H_

#include <limits.h>
#include <signal.h>
#include <pthread.h>
#include "common/collections/list.h"
#include "common/utils/sockets.h"
#include "operations.h"
#include "common/utils/log.h"

typedef struct t_disk {
	uint32_t id;
	char name[13];
	pthread_t thread;
	t_list *operations;
	t_log *log;
	t_socket_client *client;
	volatile sig_atomic_t pendings; //TODO: Revisar de poner un semaphore para hacer esta operacion segura.
	uint32_t offsetlimit;
} t_disk;

void disks_init(void);
t_disk *disks_register(char *name, t_socket_client *client, t_list *waiting, t_log *log);
t_disk *disks_getidledisk(uint32_t offset);
int disks_size(void);
void disks_foreach(void(*closure)(void*));
void disks_remove(t_disk *disk);
void disks_destroy(t_disk *disk);
void disks_verifystate();

#endif

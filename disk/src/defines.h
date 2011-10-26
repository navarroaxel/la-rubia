#ifndef DEFINES_H_
#define DEFINES_H_

#include <stdbool.h>
#include "common/nipc.h"

#define CONSOLE_INFO 1
#define CONSOLE_CLEAN 2
#define CONSOLE_TRACE 3

typedef uint8_t t_sector[DISK_SECTOR_SIZE];

typedef struct t_disk_operation {
	bool read;
	uint32_t offset;
	t_sector data;
	uint8_t result;
	t_socket_client *client;
} t_disk_operation;

struct t_disk_config {
	char path[255];
	void *diskFile;
};

#endif

#ifndef DEFINES_H_
#define DEFINES_H_

#include <stdbool.h>
#include "common/nipc.h"

#define CONSOLE_INFO 1
#define CONSOLE_CLEAN 2
#define CONSOLE_TRACE 3

typedef uint8_t t_sector[DISK_SECTOR_SIZE];

typedef struct t_location {
	uint16_t cylinder;
	uint16_t sector;
} __attribute__ ((packed)) t_location;

typedef struct t_headtrace_cylinder {
	uint16_t start;
	uint16_t end;
	int8_t step;
} __attribute__ ((packed)) t_headtrace_cylinder;

typedef struct t_headtrace {
	t_location current;
	t_location requested;
	t_headtrace_cylinder cylinder_path1;
	t_headtrace_cylinder cylinder_path2;
	uint16_t sectors;
	uint32_t time;
	uint8_t hasnext;
	t_location next;
} __attribute__ ((packed)) t_headtrace;

typedef struct t_disk_operation {
	bool read;
	bool trace;
	uint32_t offset;
	t_sector data;
	uint8_t result;
	t_headtrace *headtrace;
	t_socket_client *client;
} t_disk_operation;

struct t_disk_config {
	char path[255];
	void *diskFile;
};

#endif

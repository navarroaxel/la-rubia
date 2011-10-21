#ifndef CONSOLE_H_
#define CONSOLE_H_

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include "common/utils/sockets.h"
#include "head.h"

#define SOCKET_UNIX_PATH "/tmp/lol"

void init_console(void);
void *console(void *args);

#endif

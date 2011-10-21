#ifndef HEADHELPER_H_
#define HEADHELPER_H_

#include <stdint.h>
#include "common/utils/config.h"
#define SOCKET_UNIX_PATH "/tmp/lol"

int islimitcylinder(int cylinder);
int islimitsector(int sector);
uint16_t getcylinder(uint32_t offset);
uint16_t getsector(uint32_t offset);

#endif

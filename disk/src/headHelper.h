#ifndef HEADHELPER_H_
#define HEADHELPER_H_

#include <stdint.h>
#include "common/utils/config.h"
#include "common/collections/blist.h"
#include "common/utils/log.h"
#include "location.h"
#include "defines.h"
#include "headtrace.h"

#define SOCKET_UNIX_PATH "/tmp/lol"

int getsectortime();
int islimitcylinder(int cylinder);
int getlimitcylinder(void);
int islimitsector(int sector);
int getlimitsector(void);
uint16_t getcylinder(uint32_t offset);
uint16_t getsector(uint32_t offset);
uint32_t getoffset(uint16_t cylinder, uint16_t sector);
void enqueueOperation(t_blist *waiting, t_disk_operation *op);
void waiting_log(t_blist *waiting, t_log *log);
void inprogress_log(t_list *inprogress, t_log *log);

#endif

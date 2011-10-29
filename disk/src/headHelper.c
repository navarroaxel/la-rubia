#include "headHelper.h"

extern config_disk *config;
int getsectortime(){
	return (int)(1 / (config->rpm * config->sectors / 6e4));
}
int islimitcylinder(int cylinder) {
	return cylinder == config->cylinders - 1;
}

int getlimitcylinder(void) {
	return config->cylinders - 1;
}

int islimitsector(int sector) {
	return sector == config->sectors - 1;
}

int getlimitsector(void) {
	return config->sectors - 1;
}

uint16_t getcylinder(uint32_t offset) {
	return offset / (config->heads * config->sectors);
}

uint16_t getsector(uint32_t offset) {
	return offset % config->sectors;
}

uint32_t getoffset(uint16_t cylinder, uint16_t sector){
	return cylinder * config->sectors + sector;
}

void enqueueOperation(t_blist *waiting, t_disk_operation *op) {
	//encola la operacion en base al algoritmo de R&W utilizado.
	int enqueueDiskOperation(void *data) {
		return getcylinder(op->offset) >= getcylinder(((t_disk_operation *) data)->offset);
	}
	collection_blist_put(waiting, op, &enqueueDiskOperation);
}

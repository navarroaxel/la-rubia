#include "headHelper.h"

extern config_disk * config;
int islimitcylinder(int cylinder) {
	//TODO: cambiar por archivo de config.
	return cylinder == config->cylinder - 1;
}

int islimitsector(int sector) {
	//TODO: cambiar por archivo de config.
	return sector == config->sector - 1;
}

uint16_t getcylinder(uint32_t offset) {
	//TODO: Tomar consts de config file.
	return offset / (config->head * config->sector); //head * sector
}

uint16_t getsector(uint32_t offset) {
	return offset % config->sector;
}

#include "headHelper.h"

extern config_disk *config;
int islimitcylinder(int cylinder) {
	//TODO: cambiar por archivo de config.
	return cylinder == config->cylinders - 1;
}

int islimitsector(int sector) {
	//TODO: cambiar por archivo de config.
	return sector == config->sectors - 1;
}

uint16_t getcylinder(uint32_t offset) {
	//TODO: Tomar consts de config file.
	return offset / (config->heads * config->sectors);
}

uint16_t getsector(uint32_t offset) {
	return offset % config->sectors;
}

uint32_t getoffset(uint16_t cylinder, uint16_t sector){
	return cylinder * config->sectors + sector;
}

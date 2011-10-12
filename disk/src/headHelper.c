#include "headHelper.h"

int islimitcylinder(int cylinder) {
	//TODO: cambiar por archivo de config.
	return cylinder == 32768 - 1;
}

int islimitsector(int sector) {
	//TODO: cambiar por archivo de config.
	return sector == 32 - 1;
}

uint16_t getcylinder(uint32_t offset) {
	//TODO: Tomar consts de config file.
	return offset / (1 * 32); //head * sector
}

uint16_t getsector(uint32_t offset) {
	return offset % 32;
}

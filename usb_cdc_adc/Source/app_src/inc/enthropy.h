#ifndef ENTHROPY_H
#define ENTHROPY_H

#include "termometer_interface.h"
#include "L3GD20_interface.h"

void getRandomData(uint8_t *buffer, uint16_t size);
void getRandomData2(uint8_t *buffer, uint16_t size, double minEnthropy);
	
	
#endif

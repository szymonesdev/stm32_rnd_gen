#ifndef ENTHROPY_H
#define ENTHROPY_H

#include <stdio.h>

#include "termometer_interface.h"
#include "L3GD20_interface.h"


uint8_t* getRandomData(uint16_t size);
uint8_t* getRandomData2(uint16_t size, double minEnthropy);
	
	
#endif

#ifndef ENTHROPY_H
#define ENTHROPY_H

#include "termometer_interface.h"
#include "L3GD20_interface.h"

typedef struct {
	double enthropy;
	uint8_t* randomData;
}ClientData;

ClientData getRandomData( uint16_t requestedSize, double minEnthropy);
	
#endif

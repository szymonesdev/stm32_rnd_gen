#ifndef TEMP_INTERFACE_H
#define TEMP_INTERFACE_H

#include "stm32f4xx_hal.h"
#include <stdio.h>

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

void Termometer_initialize(void);
uint16_t Termometer_getADCReading(void);

void MX_ADC1_Init(void);
void MX_DMA_Init(void) ;

#endif



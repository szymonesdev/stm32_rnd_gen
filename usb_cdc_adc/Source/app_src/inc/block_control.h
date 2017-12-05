#ifndef BCTRL_H
#define BCTRL_H

#include <stdio.h>

#include "stm32f4xx_hal.h"

#include "termometer_interface.h"
#include "L3GD20_interface.h"

const uint16_t BLOCK_T_DATA_SIZE = 100; // multiple of 4
const uint8_t CLUSTER_T_BLOCK_CNT = 40;

enum BlockState_t { BLOCK_READY, BLOCK_USED };

typedef struct {
	uint8_t data[BLOCK_T_DATA_SIZE];
} Block_t;

typedef struct {
	Block_t blocks[CLUSTER_T_BLOCK_CNT];
	enum BlockState_t blockState[CLUSTER_T_BLOCK_CNT];
} BlockCluster_t;

void installBlockCluster(BlockCluster_t *blockClusterPtr);

uint8_t refillBLock(void);

uint8_t getBytes(uint16_t byteCnt);

uint8_t refillAndGetBytes(uint16_t bytes);

#endif
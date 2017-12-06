#ifndef BCTRL_H
#define BCTRL_H

#include <stdio.h>

#include "stm32f4xx_hal.h"

#include "termometer_interface.h"
#include "L3GD20_interface.h"

#define BLOCK_T_DATA_SIZE 				100 // multiple of 4
#define CLUSTER_T_BLOCK_CNT  			40

enum BlockState_t { BLOCK_READY, BLOCK_USED, BLOCK_LOCKED };

typedef struct {
	uint8_t data[BLOCK_T_DATA_SIZE];
} Block_t;

typedef struct {
	Block_t blocks[CLUSTER_T_BLOCK_CNT];
	enum BlockState_t blockState[CLUSTER_T_BLOCK_CNT];
} BlockCluster_t;

// Install externally stored cluster data structure
void installBlockCluster(BlockCluster_t *blockClusterPtr);

// Call to refill one block worth of data
uint8_t refillBlock(void);

// Get ptr to data, if returns 1
// Locks blocks, unlockBlocks() to unlock 
uint8_t getBytes(uint16_t byteCnt, uint8_t **dataStartPtr);

// Refill as many as needed and return ptr to data, if returns 1
// Locks blocks, unlockBlocks() to unlock 
uint8_t refillAndGetBytes(uint16_t bytes, uint8_t **dataStartPtr);

// Unlock blocks in use, will be refilled in refillBlock() calls
void unlockBlocks(void);

#endif

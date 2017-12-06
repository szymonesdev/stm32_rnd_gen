#include "block_control.h"

static BlockCluster_t *cluster;

//static uint8_t head = 0; // Returned bytes must be in sequence in memory
//static uint8_t tail = 0; // so queue implementation is a bit more complex, quick hack below

	
static uint8_t invalidateBlockSeq(uint8_t beg, uint8_t len){
	for (int i=beg; i < beg + len - 1; ++i){
		if (i >= CLUSTER_T_BLOCK_CNT) return 0;
		cluster->blockState[i] = BLOCK_USED;
	}
	return 1;
}

static uint8_t lockBlockSeq(uint8_t beg, uint8_t len){
	for (int i=beg; i < beg + len - 1; ++i){
		if (i >= CLUSTER_T_BLOCK_CNT) return 0;
		cluster->blockState[i] = BLOCK_LOCKED;
	}
	return 1;
}
	
// Uses delay
// Block size = multiple of 4
static uint8_t readDataAndFillBlock(Block_t *block){
	L3GD20_XYZ_data_t xyz_data;
	uint8_t *buffer = block->data;
	uint16_t block_size = BLOCK_T_DATA_SIZE / 4;
	while (block_size--){
		uint8_t x, y, z, t;
		x = y = z = t = 0;
		
		for (int i = 0; i < 4; i++){
			L3GD20_readXYZ(&xyz_data);
			uint16_t termOut = Termometer_getADCReading();
			
			x |= (xyz_data.x_lsb & 0x03); x <<= 2;
			y |= (xyz_data.y_lsb & 0x03); y <<= 2;
			z |= (xyz_data.z_lsb & 0x03); z <<= 2;
			t |= (termOut & 0x03); t <<= 2;
			
			HAL_Delay(1);
		}
		
		*buffer++ = x;
		*buffer++ = y;
		*buffer++ = z;
		*buffer++ = t;
	}
	return 1; // Error output possibly here
}

static uint8_t findReadyBLockSequence(uint8_t len){
	uint8_t beg = 0, end = 0;
	while (end != CLUSTER_T_BLOCK_CNT){
		beg = end;
		while (cluster->blockState[beg] != BLOCK_READY) {
			if (beg == CLUSTER_T_BLOCK_CNT) return 0;
			beg++;
		}
		while (cluster->blockState[end] == BLOCK_READY) {
			if (end == CLUSTER_T_BLOCK_CNT) return 0;
			if (end - beg == len) return beg;
			end++;
		}
	}
//	for (int i=0; i < CLUSTER_T_BLOCK_CNT - len; ++i){
//		
//		if (cluster->blockState[i] == BLOCK_READY 
//			&& cluster->blockState[i + len] == BLOCK_READY) // #TODO this will return locked blocks if: RLLLLLLLLLR
//			return i;
//	}
	return 0;
}

void installBlockCluster(BlockCluster_t *blockClusterPtr){
	cluster = blockClusterPtr;
	invalidateBlockSeq(0, CLUSTER_T_BLOCK_CNT);
}

uint8_t refillBLock(void) {
	for (int i=0; i < CLUSTER_T_BLOCK_CNT; ++i){
		if (cluster->blockState[i] == BLOCK_USED){
			readDataAndFillBlock(&(cluster->blocks[i]));
			cluster->blockState[i] = BLOCK_READY;
			return 1;
		}
	}
	return 0;
}

uint8_t getBytes(uint16_t byteCnt, uint8_t **dataStartPtr) {
	uint16_t minBlocksRequired = byteCnt / BLOCK_T_DATA_SIZE;
	
	if (minBlocksRequired > CLUSTER_T_BLOCK_CNT) return 0;
	if (cluster->blockState[minBlocksRequired] == BLOCK_USED) return 0;
	
	uint8_t i = findReadyBLockSequence(minBlocksRequired);
	if (i == 0) return 0;
	
	*dataStartPtr = cluster->blocks[i].data;
	lockBlockSeq(i, minBlocksRequired);
	
	return 1;
}

uint8_t refillAndGetBytes(uint16_t byteCnt, uint8_t **dataStartPtr) {
	uint16_t minBlocksRequired = byteCnt / BLOCK_T_DATA_SIZE + 1;
	
	if (minBlocksRequired > CLUSTER_T_BLOCK_CNT) return 0;
	
	uint8_t i = 0;
	while ((i = findReadyBLockSequence(minBlocksRequired))) {
		refillBLock();
	}
	
	*dataStartPtr = cluster->blocks[0].data;
	
	lockBlockSeq(i, minBlocksRequired);
	
	return 1;
}

void unlockBlocks(void) {
	for (int i=0; i < CLUSTER_T_BLOCK_CNT; ++i){
		if (cluster->blockState[i] == BLOCK_LOCKED)
			cluster->blockState[i] = BLOCK_USED;
	}
}

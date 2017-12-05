#include "block_control.h"

static BlockCluster_t *cluster;
	
// Uses delay
// Block size = multiple of 4
static uint8_t fillBlock(Block_t *block){
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

void installBlockCluster(BlockCluster_t *blockClusterPtr){
	cluster = blockClusterPtr;
	for (int i=0; i < CLUSTER_T_BLOCK_CNT; ++i){
		cluster->blockState[i] = BLOCK_USED;
	}
}

uint8_t refillBLock(void) {
	for (int i=0; i < CLUSTER_T_BLOCK_CNT; ++i){
		if (cluster->blockState[i] == BLOCK_USED){
			fillBlock(&(cluster->blocks[i]));
			return 1;
		}
	}
	return 0;
}

uint8_t getBytes(uint16_t byteCnt) {
	uint16_t minBlocksRequired = byteCnt / BLOCK_T_DATA_SIZE;
	if (
}

uint8_t refillAndGetBytes(uint16_t bytes);

#include "spi_interface.h"

#include "stm32f4xx_hal.h"
#include <stdio.h>


SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi5;

void SPI1_SELECT(void) { HAL_GPIO_WritePin(SPI1_CS_GPIO_PORT, SPI1_CS_PIN, GPIO_PIN_RESET); }
void SPI1_DESELECT(void) { HAL_GPIO_WritePin(SPI1_CS_GPIO_PORT, SPI1_CS_PIN, GPIO_PIN_SET); }

void SPI5_SELECT(void) { HAL_GPIO_WritePin(SPI5_CS_GPIO_PORT, SPI5_CS_PIN, GPIO_PIN_RESET); }
void SPI5_DESELECT(void) { HAL_GPIO_WritePin(SPI5_CS_GPIO_PORT, SPI5_CS_PIN, GPIO_PIN_SET); }

void SPI1_L3GD20_WRITE_REG(uint8_t addr, uint8_t data){
	//uint8_t txData[] = { 0x80 | addr, data };
	uint8_t txData[] = { addr, data };
	SPI1_SELECT();
	HAL_SPI_Transmit(&hspi1, txData, 2, SPIx_TIMEOUT);
	SPI1_DESELECT();
}

void SPI1_L3GD20_READ_REG(uint8_t addr, uint8_t *data){
	addr |= 0x80;
	SPI1_SELECT();
	HAL_SPI_Transmit(&hspi1, &addr, 1, SPIx_TIMEOUT);
	HAL_SPI_Receive(&hspi1, data, 1, SPIx_TIMEOUT);
	SPI1_DESELECT();
}

void SPI5_L3GD20_WRITE_REG(uint8_t addr, uint8_t data){
	//uint8_t txData[] = { 0x80 | addr, data };
	uint8_t txData[] = { addr, data };
	SPI5_SELECT();
	HAL_SPI_Transmit(&hspi5, txData, 2, SPIx_TIMEOUT);
	SPI5_DESELECT();
}

void SPI5_L3GD20_READ_REG(uint8_t addr, uint8_t *data){
	addr |= 0x80;
	SPI5_SELECT();
	HAL_SPI_Transmit(&hspi5, &addr, 1, SPIx_TIMEOUT);
	HAL_SPI_Receive(&hspi5, data, 1, SPIx_TIMEOUT);
	SPI5_DESELECT();
}


void SPI5_L3GD20_READ_XYZ(uint8_t *dataBuffer) {
	uint8_t addr = L3GD20_REG_OUT_X_L | 0x80 | 0x40; // Read mode, address increment
	SPI5_SELECT();
	HAL_SPI_Transmit(&hspi5, &addr, 1, SPIx_TIMEOUT);
	HAL_SPI_Receive(&hspi5, dataBuffer, 6, SPIx_TIMEOUT);
	SPI5_DESELECT();
}

/*
	---------------- SPI 1 init ----------------
*/	
void MX_SPI1_Init(void){
	
	// SPI1 init
	
	hspi1.Instance = SPI1;
	
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
	hspi1.Init.Direction         = SPI_DIRECTION_2LINES;
	hspi1.Init.CLKPhase          = SPI_PHASE_1EDGE;
	hspi1.Init.CLKPolarity       = SPI_POLARITY_LOW;
	hspi1.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
	hspi1.Init.CRCPolynomial     = 7;
	hspi1.Init.DataSize          = SPI_DATASIZE_8BIT;
	hspi1.Init.FirstBit          = SPI_FIRSTBIT_MSB;
	hspi1.Init.NSS               = SPI_NSS_SOFT;
	hspi1.Init.TIMode            = SPI_TIMODE_DISABLED;
  
	hspi1.Init.Mode = SPI_MODE_MASTER;

	if(HAL_SPI_Init(&hspi1) != HAL_OK){
		/* Initialization Error */
		while(1);
		//_Error_Handler(__FILE__, __LINE__);
	}
	
	// GPIO init - CS
	GPIO_InitTypeDef  GPIO_InitStruct;
	GPIO_InitStruct.Pin       = SPI1_CS_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
  
	HAL_GPIO_Init(SPI1_CS_GPIO_PORT, &GPIO_InitStruct);
	
	HAL_GPIO_WritePin(SPI1_CS_GPIO_PORT, SPI1_CS_PIN, GPIO_PIN_SET);
}

/*
	---------------- SPI 5 init ----------------
*/	
void MX_SPI5_Init(void){
	
	// SPI1 init
	
	hspi5.Instance = SPI5;
	
	hspi5.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
	hspi5.Init.Direction         = SPI_DIRECTION_2LINES;
	hspi5.Init.CLKPhase          = SPI_PHASE_1EDGE;
	hspi5.Init.CLKPolarity       = SPI_POLARITY_LOW;
	hspi5.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
	hspi5.Init.CRCPolynomial     = 7;
	hspi5.Init.DataSize          = SPI_DATASIZE_8BIT;
	hspi5.Init.FirstBit          = SPI_FIRSTBIT_MSB;
	hspi5.Init.NSS               = SPI_NSS_SOFT;
	hspi5.Init.TIMode            = SPI_TIMODE_DISABLED;
  
	hspi5.Init.Mode = SPI_MODE_MASTER;

	if(HAL_SPI_Init(&hspi5) != HAL_OK){
		/* Initialization Error */
		while(1);
		//_Error_Handler(__FILE__, __LINE__);
	}
	
	// GPIO init - CS
	GPIO_InitTypeDef  GPIO_InitStruct;
	GPIO_InitStruct.Pin       = SPI5_CS_PIN;
	GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull      = GPIO_PULLUP;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
  
	HAL_GPIO_Init(SPI5_CS_GPIO_PORT, &GPIO_InitStruct);
	
	HAL_GPIO_WritePin(SPI5_CS_GPIO_PORT, SPI5_CS_PIN, GPIO_PIN_SET);
}


// -------------------------------------------------------------

/*
		------------------ SPI HAL MSP init/deinit -----------------
*/

void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi){
	if (hspi->Instance == SPI1){
			GPIO_InitTypeDef  GPIO_InitStruct;
	  
			SPI1_SCK_GPIO_CLK_ENABLE();
			SPI1_MISO_GPIO_CLK_ENABLE();
			SPI1_MOSI_GPIO_CLK_ENABLE();
		  /* Enable SPI clock */
			SPI1_CLK_ENABLE();
		  
			GPIO_InitStruct.Pin       = SPI1_SCK_PIN;
			GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
			GPIO_InitStruct.Pull      = GPIO_PULLDOWN;
			GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
			GPIO_InitStruct.Alternate = SPI1_SCK_AF;
		  
			HAL_GPIO_Init(SPI1_SCK_GPIO_PORT, &GPIO_InitStruct);
		  
			/* SPI MOSI GPIO pin configuration  */
			GPIO_InitStruct.Pin = SPI1_MOSI_PIN;
			GPIO_InitStruct.Pull      = GPIO_PULLUP;
			GPIO_InitStruct.Alternate = SPI1_MOSI_AF;
			 
			HAL_GPIO_Init(SPI1_MOSI_GPIO_PORT, &GPIO_InitStruct); 

			/* SPI MISO GPIO pin configuration  */
			GPIO_InitStruct.Pin = SPI1_MISO_PIN;
			GPIO_InitStruct.Alternate = SPI1_MISO_AF;
		  
			HAL_GPIO_Init(SPI1_MISO_GPIO_PORT, &GPIO_InitStruct);
	}
	
	if (hspi->Instance == SPI5){
			GPIO_InitTypeDef  GPIO_InitStruct;
	  
			SPI5_SCK_GPIO_CLK_ENABLE();
			SPI5_MISO_GPIO_CLK_ENABLE();
			SPI5_MOSI_GPIO_CLK_ENABLE();
		  /* Enable SPI clock */
			SPI5_CLK_ENABLE();
		  
			GPIO_InitStruct.Pin       = SPI5_SCK_PIN;
			GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
			GPIO_InitStruct.Pull      = GPIO_PULLDOWN;
			GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
			GPIO_InitStruct.Alternate = SPI5_SCK_AF;
		  
			HAL_GPIO_Init(SPI5_SCK_GPIO_PORT, &GPIO_InitStruct);
		  
			/* SPI MOSI GPIO pin configuration  */
			GPIO_InitStruct.Pin = SPI5_MOSI_PIN;
			GPIO_InitStruct.Pull      = GPIO_PULLUP;
			GPIO_InitStruct.Alternate = SPI5_MOSI_AF;
			 
			HAL_GPIO_Init(SPI5_MOSI_GPIO_PORT, &GPIO_InitStruct); 

			/* SPI MISO GPIO pin configuration  */
			GPIO_InitStruct.Pin 		  = SPI5_MISO_PIN;
			GPIO_InitStruct.Alternate = SPI5_MISO_AF;
		  
			HAL_GPIO_Init(SPI5_MISO_GPIO_PORT, &GPIO_InitStruct);
	}
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
	if (hspi->Instance == SPI1){
		  SPI1_FORCE_RESET();
		  SPI1_RELEASE_RESET();

		  HAL_GPIO_DeInit(SPI1_SCK_GPIO_PORT, SPI1_SCK_PIN);
		  HAL_GPIO_DeInit(SPI1_MISO_GPIO_PORT, SPI1_MISO_PIN);
		  HAL_GPIO_DeInit(SPI1_MOSI_GPIO_PORT, SPI1_MOSI_PIN);
	}
	
	if (hspi->Instance == SPI5){
		  SPI5_FORCE_RESET();
		  SPI5_RELEASE_RESET();

		  HAL_GPIO_DeInit(SPI5_SCK_GPIO_PORT, SPI5_SCK_PIN);
		  HAL_GPIO_DeInit(SPI5_MISO_GPIO_PORT, SPI5_MISO_PIN);
		  HAL_GPIO_DeInit(SPI5_MOSI_GPIO_PORT, SPI5_MOSI_PIN);
	}

}

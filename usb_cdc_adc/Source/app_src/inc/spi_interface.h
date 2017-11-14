#ifndef SPI_INTERFACE_H
#define SPI_INTERFACE_H

#include "stm32f4xx_hal.h"
#include <stdio.h>

/*
			L3GD20 reg addr defines
*/

#define L3GD20_REG_WHOAMI_Val				0xD4

#define L3GD20_REG_WHOAMI					0x0F
#define L3GD20_REG_CTRL_1					0x20
#define L3GD20_REG_CTRL_2					0x21
#define L3GD20_REG_CTRL_3					0x22
#define L3GD20_REG_CTRL_4					0x23
#define L3GD20_REG_CTRL_5					0x24
#define L3GD20_REG_STATUS					0x27
#define L3GD20_REG_OUT_X_L					0x28
#define L3GD20_REG_OUT_X_H					0x29
#define L3GD20_REG_OUT_Y_L					0x2A
#define L3GD20_REG_OUT_Y_H					0x2B
#define L3GD20_REG_OUT_Z_L					0x2C
#define L3GD20_REG_OUT_Z_H					0x2D

#define L3GD20_CTRL_1_OR_Pos				6u			// Output rate 		[7:6]
#define L3GD20_CTRL_1_BW_Pos				4u			// Bandwidth   		[5:4]
#define L3GD20_CTRL_1_PD_Pos				3u			// Power-down mode 	[3]
#define L3GD20_CTRL_1_Zen_Pos				2u			// Z axis enable 		[2]
#define L3GD20_CTRL_1_Yen_Pos				1u			// Y axis enable 		[1]
#define L3GD20_CTRL_1_Xen_Pos				0u			// X axis enable 		[0]

/*
			SPI interface defines
*/

#define SPIx_TIMEOUT			3000

#define SPI1_CLK_ENABLE()                __SPI1_CLK_ENABLE()
#define SPI1_SCK_GPIO_CLK_ENABLE()       __GPIOA_CLK_ENABLE()
#define SPI1_MISO_GPIO_CLK_ENABLE()      __GPIOA_CLK_ENABLE() 
#define SPI1_MOSI_GPIO_CLK_ENABLE()      __GPIOA_CLK_ENABLE() 

#define SPI1_FORCE_RESET()               __SPI1_FORCE_RESET()
#define SPI1_RELEASE_RESET()             __SPI1_RELEASE_RESET()

/* Definition for SPI1 Pins */
#define SPI1_CS_PIN								GPIO_PIN_3
#define SPI1_CS_GPIO_PORT						GPIOE
#define SPI1_SCK_PIN                     GPIO_PIN_5
#define SPI1_SCK_GPIO_PORT               GPIOA
#define SPI1_SCK_AF                      GPIO_AF5_SPI1
#define SPI1_MISO_PIN                    GPIO_PIN_6
#define SPI1_MISO_GPIO_PORT              GPIOA
#define SPI1_MISO_AF                     GPIO_AF5_SPI1
#define SPI1_MOSI_PIN                    GPIO_PIN_7
#define SPI1_MOSI_GPIO_PORT              GPIOA
#define SPI1_MOSI_AF                     GPIO_AF5_SPI1


#define SPI5_CLK_ENABLE()                __SPI5_CLK_ENABLE()
#define SPI5_SCK_GPIO_CLK_ENABLE()       __GPIOF_CLK_ENABLE()
#define SPI5_MISO_GPIO_CLK_ENABLE()      __GPIOF_CLK_ENABLE() 
#define SPI5_MOSI_GPIO_CLK_ENABLE()      __GPIOF_CLK_ENABLE() 

#define SPI5_FORCE_RESET()               __SPI5_FORCE_RESET()
#define SPI5_RELEASE_RESET()             __SPI5_RELEASE_RESET()

/* Definition for SPI5 Pins */
#define SPI5_CS_PIN								GPIO_PIN_1
#define SPI5_CS_GPIO_PORT						GPIOC
#define SPI5_SCK_PIN                     GPIO_PIN_7
#define SPI5_SCK_GPIO_PORT               GPIOF
#define SPI5_SCK_AF                      GPIO_AF5_SPI5
#define SPI5_MISO_PIN                    GPIO_PIN_8
#define SPI5_MISO_GPIO_PORT              GPIOF
#define SPI5_MISO_AF                     GPIO_AF5_SPI5
#define SPI5_MOSI_PIN                    GPIO_PIN_9
#define SPI5_MOSI_GPIO_PORT              GPIOF
#define SPI5_MOSI_AF                     GPIO_AF5_SPI5

extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi5;

/*
	SPI functions
*/
void SPI1_SELECT(void);
void SPI1_DESELECT(void);

void SPI5_SELECT(void);
void SPI5_DESELECT(void);

void SPI1_L3GD20_WRITE_REG(uint8_t addr, uint8_t data);
void SPI1_L3GD20_READ_REG(uint8_t addr, uint8_t *data);

void SPI5_L3GD20_WRITE_REG(uint8_t addr, uint8_t data);
void SPI5_L3GD20_READ_REG(uint8_t addr, uint8_t *data);

void SPI5_L3GD20_READ_XYZ(uint8_t *dataBuffer);

void MX_SPI1_Init(void);
void MX_SPI5_Init(void);

#endif

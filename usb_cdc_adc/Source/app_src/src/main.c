#include "stm32f4xx_hal.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include <stdio.h>

#include "Board_LED.h"

#include "spi_interface.h"


static const uint32_t LED_GREEN = 0;
static const uint32_t LED_RED = 1;

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

void SystemClock_Config(void);
void _Error_Handler(char * file, int line);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_DMA_Init(void) ;

uint8_t MEMS_XYZ[6];
uint16_t ADC_DUMP[2];

int32_t DELAY_A;
int32_t DELAY_B;
char NL[2]= {'\n', '\r'};
volatile char TX_DATA[256];
volatile uint32_t len;
uint32_t TERM_BIT0_1;
uint32_t TERM_BIT0_0;
uint32_t TERM_BIT1_1;
uint32_t TERM_BIT1_0;

int main(){

	HAL_Init();
   SystemClock_Config();
	MX_GPIO_Init();
   MX_USB_DEVICE_Init();
	MX_DMA_Init();
	MX_ADC1_Init();
	
	//MX_SPI1_Init();
	MX_SPI5_Init();
	
	LED_Initialize();
	
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC_DUMP, 2);
	
//	const char *helloStr = "Hello\r\n";
//	CDC_Transmit_HS( (uint8_t*)helloStr, len );
	
	/*
	 L3GD20 init, register check
	*/
	SPI5_L3GD20_WRITE_REG(L3GD20_REG_CTRL_1, 
		(0x03 << L3GD20_CTRL_1_OR_Pos) | 
		(0x03 << L3GD20_CTRL_1_BW_Pos) | 
		(0x01 << L3GD20_CTRL_1_PD_Pos) |
		(0x01 << L3GD20_CTRL_1_Xen_Pos) |
		(0x01 << L3GD20_CTRL_1_Zen_Pos) |
		(0x01 << L3GD20_CTRL_1_Yen_Pos)
	);
	uint8_t rx = 0; 
	SPI5_L3GD20_READ_REG(L3GD20_REG_WHOAMI, &rx);
	if (rx == L3GD20_REG_WHOAMI_Val) LED_On(LED_GREEN);
	
	
	while(1){
		
		LED_Off(LED_RED);
		
		DELAY_A = 1000;
		
		while( (DELAY_A--) > 0 ){
			if( ADC_DUMP[0] & 0x1 )
				TERM_BIT0_1++;
			else
				TERM_BIT0_0++;
			
			if( ADC_DUMP[0] & 0x2 )
				TERM_BIT1_1++;
			else
				TERM_BIT1_0++;
			
			DELAY_B = 1000000;
			while( (DELAY_B--) > 0 ){}
		}
		
		LED_On(LED_RED);
		
		SPI5_L3GD20_READ_XYZ(MEMS_XYZ);
		
		len = sprintf(TX_DATA, 
			"TEMPERATURE MEASURE \n\r BIT0 0: %d \n\r BIT0 1: %d \n\r BIT1 0: %d \n\r BIT1 1: %d \n\r", 
			TERM_BIT0_0, TERM_BIT0_1, TERM_BIT1_0, TERM_BIT1_1);
		CDC_Transmit_HS( (uint8_t*)TX_DATA, len );
		
		len = sprintf(TX_DATA, 
			"MEMS MEASURE X/Y/Z %#010x %#010x %#010x \n\r", 
			((uint16_t)MEMS_XYZ[1] << 8) | MEMS_XYZ[0], 
			((uint16_t)MEMS_XYZ[3] << 8) | MEMS_XYZ[2],
			((uint16_t)MEMS_XYZ[5] << 8) | MEMS_XYZ[4]
		);
		CDC_Transmit_HS( (uint8_t*)TX_DATA, len );
	}
	
	return 0;
}

 static void MX_GPIO_Init(void)
{
  /* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
	// SPI5
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
}
/*
void DMA2_Stream0_IRQHandler(){
	//HAL_NVIC_DisableIRQ(DMA2_Stream0_IRQn);
	DELAY= 10000000;
	while(DELAY--){}
	len = sprintf(NUMB, "%d \n\r", ADC_DUMP[0]);
	CDC_Transmit_HS( (uint8_t*)NUMB, len );
	len = sprintf(NUMB, "%d \n\r", ADC_DUMP[1]);
	CDC_Transmit_HS( (uint8_t*)NUMB, len );
	//HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
}*/

static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA2_Stream0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 1, 0);
 // HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);

}

static void MX_ADC1_Init(void)
{

  ADC_ChannelConfTypeDef sConfig;

    /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
    */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV8;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time. 
    */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
 
  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
 
  /* The voltage scaling allows optimizing the power consumption when the
     device is clocked below the maximum system frequency (see datasheet). */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
 
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);
 
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 |
                                RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(hadc->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspInit 0 */

  /* USER CODE END ADC1_MspInit 0 */
    /* Peripheral clock enable */
    __HAL_RCC_ADC1_CLK_ENABLE();
  
    /**ADC1 GPIO Configuration    
    PA1     ------> ADC1_IN1 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* ADC1 DMA Init */
    /* ADC1 Init */
    hdma_adc1.Instance = DMA2_Stream0;
    hdma_adc1.Init.Channel = DMA_CHANNEL_0;
    hdma_adc1.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode = DMA_CIRCULAR;
    hdma_adc1.Init.Priority = DMA_PRIORITY_LOW;
    hdma_adc1.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
	 
    if (HAL_DMA_Init(&hdma_adc1) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_LINKDMA(hadc,DMA_Handle,hdma_adc1);

  /* USER CODE BEGIN ADC1_MspInit 1 */

  /* USER CODE END ADC1_MspInit 1 */
  }

}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{

  if(hadc->Instance==ADC1)
  {
  /* USER CODE BEGIN ADC1_MspDeInit 0 */

  /* USER CODE END ADC1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC1_CLK_DISABLE();
  
    /**ADC1 GPIO Configuration    
    PA1     ------> ADC1_IN1 
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1);

    /* ADC1 DMA DeInit */
    HAL_DMA_DeInit(hadc->DMA_Handle);
  /* USER CODE BEGIN ADC1_MspDeInit 1 */

  /* USER CODE END ADC1_MspDeInit 1 */
  }

}

void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

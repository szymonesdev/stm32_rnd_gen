#include "stm32f4xx_hal.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include <stdio.h>

#include "Board_LED.h"

#include "termometer_interface.h"
#include "L3GD20_interface.h"

static const uint32_t LED_GREEN = 0;
static const uint32_t LED_RED = 1;

static void MX_GPIO_Init(void);
void SystemClock_Config(void);
void _Error_Handler(char * file, int line);


char NL[2]= {'\n', '\r'};
volatile char TX_DATA[256];
volatile uint32_t len;


int main(){

	/*
		Initialize start
	*/
	HAL_Init();
    SystemClock_Config();
	MX_GPIO_Init();
    MX_USB_DEVICE_Init();

	Termometer_initialize();
	L3GD20_initialize();
	
	LED_Initialize();
	/*
		Initialize stop
	*/
	
	const char *helloStr = "Hello\r\n";
	CDC_Transmit_HS( (uint8_t*)helloStr, len );
	
	L3GD20_XYZ_data_t xyz_data;
	uint16_t termval;
	
	LED_On(LED_GREEN);
	
	while(1){
		
		volatile int x = 0;
		const uint16_t OUTER_DELAY = 100;
		const uint16_t INNER_DELAY = 10000;
		for (int i = 0; i< OUTER_DELAY; ++i){
			for (int j = 0; j<INNER_DELAY; ++j)
				x += 1;
		}

		/*
			#TODO usunac ta notatke
			Te funkcje pobieraja dane odpowiednio z zyroskopu i termometru
			Skladamy wyjsciowe bajty z 4 kolejnych pomiarow, po 2 bity z kazdego
			dla termometru / 3 osi zyroskopu osobno. 
			Z zyroskopu mozna czytac z max czest. ~750Hz
			
			Trzeba w glownej petli programu napisac jakies taktowanie pomiarow,
			nie pisalem bo duzo zalezy od tego, jak bedzie wygladac 
			czesc kodu liczaca entropie i wystawiajaca bajty dla usera
		
			Przyklad skladania
			uint8_t b = 0x00 | nowe_dane & 0x03; // dane z 1 pomiaru
			petla : 3 pomiary
				b =<< 2;
				b |= nowe_dane & 0x03;
		*/
		L3GD20_readXYZ(&xyz_data);
		termval = Termometer_getADCReading();
		
		
		len = sprintf(TX_DATA, 
			"MEASURE ADC/X/Y/Z %#06x %#06x %#06x %#06x\n\r", 
			termval,
			((uint16_t)xyz_data.x_msb << 8) | xyz_data.x_lsb, 
			((uint16_t)xyz_data.y_msb << 8) | xyz_data.y_lsb,
			((uint16_t)xyz_data.z_msb << 8) | xyz_data.z_lsb
		);
		
		
		CDC_Transmit_HS( (uint8_t*)TX_DATA, len );
	}
	
	return 0;
}

 static void MX_GPIO_Init(void)
{
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
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



void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

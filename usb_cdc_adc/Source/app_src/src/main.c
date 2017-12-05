/*

	MAIN

*/

#include <stdio.h>

#include "stm32f4xx_hal.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

//#include "Board_LED.h"

#include "termometer_interface.h"
#include "L3GD20_interface.h"
#include "enthropy.h"

//static const uint32_t LED_GREEN = 0;
//static const uint32_t LED_RED = 1;

#define MAX_INPUT_DIGITS 3
#define MAX_INPUT_NUMBER 512

static void MX_GPIO_Init(void);
void SystemClock_Config(void);
void _Error_Handler(char * file, int line);

char BUFF_INPUT[]= "\n\rTRUE RANDOM GENERATOR, input number of bytes to be generated: ";
char BUFF_ERR[]= "\n\rInput error, numbers up to 4096 are allowed\n\rTRUE RANDOM GENERATOR, input number of bytes to be generated:";
char BUFF_INERR[]= "\n\rInput error";
char NL[2]= {'\n', '\r'};
char INPUT_DIGIT[ MAX_INPUT_DIGITS ];

static uint8_t FLAG_CLIENT_REQUEST = 0;
static uint32_t REQUESTED_BYTES, CURRENT_DIGIT; // new usb read, Witold

volatile char TX_DATA[256];
volatile uint32_t len;

ClientData cdata;

static void usbWaitBusy(void){
	USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceHS.pClassData;
	while (hcdc->TxState != 0);
}

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
	
	//LED_Initialize();
	/*
		Initialize stop
	*/
	
	L3GD20_XYZ_data_t xyz_data;
	uint16_t termval;
	
	//LED_On(LED_GREEN);
	
	const uint16_t BYTECNT = 550;
	cdata = getRandomData(BYTECNT, 0.0);
	
	CDC_Transmit_HS( (uint8_t*)BUFF_INPUT, strlen( BUFF_INPUT ) );
	
	while(1){

//		HAL_Delay(2);
//		L3GD20_readXYZ(&xyz_data);
//		termval = Termometer_getADCReading();
//		len = sprintf(TX_DATA, 
//			"MEASURE ADC/X/Y/Z %#06x %#06x %#06x %#06x\n\r", 
//			termval,
//			((uint16_t)xyz_data.x_msb << 8) | xyz_data.x_lsb, 
//			((uint16_t)xyz_data.y_msb << 8) | xyz_data.y_lsb,
//			((uint16_t)xyz_data.z_msb << 8) | xyz_data.z_lsb
//		);
//		CDC_Transmit_HS( (uint8_t*)TX_DATA, len );
		
		HAL_Delay(200);
		
		if (FLAG_CLIENT_REQUEST) {
			
			len = sprintf(TX_DATA, "\r\nRequested %d byte(s)\r\n", REQUESTED_BYTES);
			CDC_Transmit_HS( (uint8_t*)TX_DATA, len );
			
			if( REQUESTED_BYTES < 4097 ){	
				cdata = getRandomData(REQUESTED_BYTES, 0.0);
				int len1;
				char tab1[4];
	
				for( int i= 0; i < REQUESTED_BYTES; i++ ){
				   len1= sprintf( tab1, "%d", cdata.randomData[i] );
					 usbWaitBusy();
					 CDC_Transmit_HS( cdata.randomData, len1 );	
				}

				CDC_Transmit_HS( (uint8_t*)BUFF_INPUT, strlen( BUFF_INPUT ) );
				usbWaitBusy();
			}
			else {
				usbWaitBusy();
				CDC_Transmit_HS( (uint8_t*)BUFF_ERR, strlen( BUFF_ERR ) );	
				usbWaitBusy();

			}
			
			FLAG_CLIENT_REQUEST = REQUESTED_BYTES = CURRENT_DIGIT = 0;
		}
		
		
	}
	
	return 0;
}

static uint32_t pow10(uint32_t pow){
	uint32_t res= 1;
	
	for(int i= 0; i < pow; i++)
	  res*= 10;
	
	return res;
}

void rgen_userInput(uint8_t* buf, uint32_t *len)
{
	if (FLAG_CLIENT_REQUEST) return;
	
  if( CURRENT_DIGIT < MAX_INPUT_DIGITS+1 ){
		if( buf[0] == '\n' || buf[0] == '\r'){
			  CDC_Transmit_HS( (uint8_t *)NL, 2 );
				for(int i= 0; i < CURRENT_DIGIT; i++)
				  REQUESTED_BYTES+= ( INPUT_DIGIT[ CURRENT_DIGIT -1 - i ] * pow10( i ) );
			  if( REQUESTED_BYTES < MAX_INPUT_NUMBER+1 )
			    FLAG_CLIENT_REQUEST = 1;
			  else
				  CDC_Transmit_HS( (uint8_t*)BUFF_ERR, strlen( BUFF_ERR ) );	
	  }else if( ( buf[0] >= '0' ) && ( buf[0] <= '9' ) ){
	    INPUT_DIGIT[ CURRENT_DIGIT++ ]= ( (char)buf[0] - '0' );
			CDC_Transmit_HS( buf, 1 );
	  }
	}
	else{
		CURRENT_DIGIT= 0;
		REQUESTED_BYTES= 0;
		CDC_Transmit_HS( (uint8_t*)BUFF_ERR, strlen( BUFF_ERR ) );	
	}
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

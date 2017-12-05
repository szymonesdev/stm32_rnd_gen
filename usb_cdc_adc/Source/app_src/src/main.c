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

#define MAX_INPUT_DIGITS 3
#define MAX_INPUT_NUMBER 512

char BUFF_INPUT[]= "\n\rTRUE RANDOM GENERATOR, input number of bytes to be generated:\n\r";
char BUFF_ERR[]= "\n\rInput error, numbers up to MAX_INPUT_NUMBER are allowed\n\rTRUE RANDOM GENERATOR, input number of bytes to be generated:\n\r";
char BUFF_INERR[]= "\n\rInput error";
char NL[2]= {'\n', '\r'};
char INPUT_DIGIT[ MAX_INPUT_DIGITS+1 ];

static volatile uint8_t FLAG_CLIENT_REQ;
static volatile uint8_t FLAG_RXD_RD;
static uint32_t REQUESTED_BYTES, CURRENT_DIGIT; // new usb read, Witold

static volatile char TXD[256];
static volatile char RXD[256];
static volatile uint32_t RXD_LEN;

void rgen_processIn( char* buf, uint32_t len);
void rgen_processOut();

static void MX_GPIO_Init(void);
void SystemClock_Config(void);
void _Error_Handler(char * file, int line);

int main(){
	HAL_Init();
  SystemClock_Config();
	MX_GPIO_Init();
  MX_USB_DEVICE_Init();

	Termometer_initialize();
	L3GD20_initialize();
	
	CDC_Transmit_HS( (uint8_t*)BUFF_INPUT, strlen( BUFF_INPUT ) );
	
	while(1){

		if( FLAG_RXD_RD ){
		  rgen_processIn( RXD, RXD_LEN);
			FLAG_RXD_RD= 0;
		}
		
		if( FLAG_CLIENT_REQ ){
			rgen_processOut();
			FLAG_CLIENT_REQ= 0;
		}
	}
	
	return 0;
}

static void usbWaitBusy(void){
	USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceHS.pClassData;
	while (hcdc->TxState != 0){};
}

static uint32_t pow10(uint32_t pow){
	uint32_t res= 1;
	
	for(int i= 0; i < pow; i++)
	  res*= 10;
	
	return res;
}

void rgen_userInput(uint8_t* buf, uint32_t *len)
{
	if( ( FLAG_CLIENT_REQ == 0 ) && ( FLAG_RXD_RD == 0 ) ){
	  memcpy( (void*)RXD, buf, *len);
	  RXD_LEN= *len;
	
	  FLAG_RXD_RD= 1;
	}
}

void rgen_processIn( char* buf, uint32_t len){
	
	for(int j= 0; j < len; j++){
		
		if( CURRENT_DIGIT < MAX_INPUT_DIGITS+1 ){
			if( buf[j] == '\n' || buf[j] == '\r'){
				
					for(int i= 0; i < CURRENT_DIGIT; i++)
						REQUESTED_BYTES+= ( INPUT_DIGIT[ CURRENT_DIGIT -1 - i ] * pow10( i ) );
				
					if( ( REQUESTED_BYTES < MAX_INPUT_NUMBER+1 ) && ( REQUESTED_BYTES ) )
						FLAG_CLIENT_REQ = 1;
					else
					  goto error;

			}else if( ( buf[j] >= '0' ) && ( buf[j] <= '9' ) ){
				INPUT_DIGIT[ CURRENT_DIGIT++ ]= ( (char)buf[j] - '0' );
				usbWaitBusy();
				CDC_Transmit_HS( (uint8_t *)&buf[j], 1 );
			}
		} else{
error:
			CURRENT_DIGIT= 0;
			REQUESTED_BYTES= 0;
			usbWaitBusy();
			CDC_Transmit_HS( (uint8_t*)BUFF_ERR, strlen( BUFF_ERR ) );
			return;
		}
		
	}
}

void rgen_processOut(){
		int len;
	  ClientData cdata;
	
	  len = sprintf( TXD, "\r\nRequested %d byte(s)\r\n", REQUESTED_BYTES );
		usbWaitBusy();
		CDC_Transmit_HS( (uint8_t*)TXD, len );
			
		cdata = getRandomData( REQUESTED_BYTES, 0.0 );
		len = sprintf( TXD, "0x");
		usbWaitBusy();
		CDC_Transmit_HS( (uint8_t*)TXD, len );	
			  
		for( int i= 0; i < REQUESTED_BYTES; i++ ){
			len = sprintf( TXD, "%02x", cdata.randomData[i] );
			usbWaitBusy();
			CDC_Transmit_HS( (uint8_t*)TXD, len );	
		}

		usbWaitBusy();
		CDC_Transmit_HS( (uint8_t*)BUFF_INPUT, strlen( BUFF_INPUT ) );
		REQUESTED_BYTES = CURRENT_DIGIT = 0;
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

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "stm32f4xx.h" 
#include "cmdhandle.h"
#include "stdint.h"
#include <stdlib.h>   
#include "unpack.h"
#include "crc.h" 
#include "shared_var.h"
//#include "shared_var.c"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

#define APP_ADDR     0x08010000u

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
 Share_t  s = {0};
  Share_t  s1 = {     //跳转到APP
    .magic = SHARE_MAGIC,
    .flag  = 0x12345678,
		.state = 0x12345678,
		.softwarenumb = 0x12345678,
		.hardwarecode = 0x12345678,
    .crc   = 0
};
	  Share_t  s2 = {
    .magic = SHARE_MAGIC,
    .flag  = 0x87654321,
		.state = 0x12345678,
		.softwarenumb = 0x12345678,
		.hardwarecode = 0x12345678,
    .crc   = 0
};
		Share_t  s3 = {
    .magic = SHARE_MAGIC,
    .flag  = 0x11223344,
		.state = 0x12345678,
		.softwarenumb = 0x12345678,
		.hardwarecode = 0x12345678,
    .crc   = 0
};		

uint8_t rx3_buffer[RX_BUFFER_SIZE];   //接上位机，串口3 422
uint8_t rx5_buffer[RX_BUFFER_SIZE];   //接高压板，串口5  232
uint8_t TXBUF0[14]= {0x55, 0xAA, 0x0E, 0x00, 0x00, 0x00,
                        0x02, 0x01, 0x05, 0x00, 0x01, 0x00, 0x00, 0x00};
	
uint16_t rx3_length;
uint16_t rx5_length;

uint8_t  rx3_flag = 0;
uint8_t  rx5_flag = 0;

uint8_t ring_data[RB_bufsz];
uint8_t cmd_buf[RB_bufsz];

uint8_t flashwr_flag=0;
uint8_t flasherase_flag=0;
uint8_t	flashover_flag=0;

												
uint32_t APP_address=APP_ADDR;
												
//uint32_t  section0=4;
										
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

//void Flash_WriteBuf(uint32_t addr, uint8_t *p, uint32_t len);
//void Flash_eraseBuf(uint32_t sector);
void Boot_SetUpdateDone(void);

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  __enable_irq();  
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();



  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_UART5_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
	
	__HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);
  __HAL_UART_ENABLE_IT(&huart5, UART_IT_IDLE);

	HAL_UART_Receive_DMA(&huart3, rx3_buffer, sizeof(rx3_buffer));
	HAL_UART_Receive_DMA(&huart5, rx5_buffer, sizeof(rx5_buffer));
	
	ringbuffer rbuf;
	decodeData g_decoder;
	ring_buf_init(&rbuf, ring_data);
	unpack_init(&g_decoder, &rbuf, cmd_buf);
	
	SHARE_Read(&s, SHARE_ADDR);
	if(s.magic==SHARE_MAGIC )  
			{
					SHARE_Erase( 3 );
					SHARE_Read(&s, SHARE_ADDR);
					if(s.magic==0xFFFFFFFF)  
					{
						SHARE_Write(&s1,3);
					}
			}
		
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	SHARE_Read(&s, SHARE_ADDR);
	if(s.flag == 0x87654321 )
				{	
					CMD_handlder_tx(TXBUF0,0x01,0x05,0x00,0x01); 
				}

  while (1)
  {
		
		if(rx3_flag ==1)  // 上位机下发数据
		{
			rx3_flag = 0;
		
			if(rx3_buffer[0] ==0x55 && rx3_buffer[7] == 0x02)  //给主控板
		   	{
					for (int i = 0; i < sizeof(rx3_buffer); i++)  ring_buf_write(&rbuf, rx3_buffer[i]);
					while (ring_buf_avail(&rbuf) > 0)             unpack_fun(&g_decoder);
					//HAL_UART_Transmit(&huart3, g_decoder.buf, rx3_length ,0xFFFF );
			   } 
		  else if(rx3_buffer[0] ==0x55 && rx3_buffer[7] == 0x03)    //给高压板
					{
						HAL_UART_Transmit_DMA(&huart5, rx3_buffer, rx3_length);  
					}	

			else 
					{
				  	CMD_handlder_tx(TXBUF0,0x05,0x01,0x00,0x00); //报错
					}
		}
	
		if( s.flag == 0x12345678 || s.flag ==0x11223344)
		{
				__disable_irq();    //禁用所有中断 
				/* 跳转到 App（地址 0x08010000） */
				typedef void (*app_fn)(void);
				uint32_t appStack = *(uint32_t *)APP_ADDR;
				app_fn   appEntry = (app_fn)*(uint32_t *)(APP_ADDR + 4);

			  /* 关中断 + 复位外设 */
				HAL_RCC_DeInit();
				HAL_DeInit();
				SysTick->CTRL = 0;
				SysTick->LOAD = 0;
				SysTick->VAL  = 0;
				__disable_irq();

				__set_MSP(appStack);  //设置主堆栈指针 
				appEntry();           //跳转 
		}
		else if( s.flag == 0x87654321 )
		{
				if( flasherase_flag==1)
				{
					flasherase_flag=0;
					SHARE_Erase(4);
					SHARE_Erase(5);
					SHARE_Erase(6);
					//flashwr_flag=1;
				}
				
				if( flashwr_flag==1)
				{
				    flashwr_flag=0;
					Flash_WriteBuf(APP_address, g_decoder.buf, ((uint32_t)g_decoder.len - 14) );//需要重新计算g_decoder.len，在unpack,c的地58行（我认为是整个数据包的长度）
				  	CMD_handlder_tx(TXBUF0,0x01,0x06,0x00,0x01); 
				}
				
					if( flashover_flag==1)
				{
				    SHARE_Erase( 3 );
						SHARE_Read(&s, SHARE_ADDR);
						if(s.magic==0xFFFFFFFF)  
						{
							SHARE_Write(&s3,3);
						}
				}
				
		}
		
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }


  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */


/* USER CODE END 4 */

/**


  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

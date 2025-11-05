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
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hardware.h"
#include "stm32f4xx.h" 

#include "cmdhandle.h"
#include "stdint.h"
#include <stdlib.h>   
#include "unpack.h"
#include "crc.h"
#include "shared_var.h"

uint8_t rx1_buffer[RX_BUFFER_SIZE];   //接电机，串口1 485
uint8_t rx4_buffer[RX_BUFFER_SIZE];   //接高压板，串口4  232
uint8_t rx3_buffer[RX_BUFFER_SIZE];   //接上位机，串口3 422
uint8_t tim2_buffer[5];
uint8_t TXBUF0[14]= {0x55, 0xAA, 0x0E, 0x00, 0x00, 0x00,
                        0x02, 0x01, 0x05, 0x00, 0x01, 0x00, 0x00, 0x00};
uint8_t TXBUF1[19]= {0x55, 0xAA, 0x13, 0x00, 0x00, 0x00,
                        0x02, 0x01, 0x05, 0x00, 0x01, 0x00,  0x00, 0x00, 0x00, 0x00 , 0x00,0x00, 0x00};
uint8_t TXBUF2[18]= {0x55, 0xAA, 0x13, 0x00, 0x00, 0x00,
                        0x02, 0x01, 0x05, 0x00, 0x01, 0x00,  0x00, 0x00, 0x00 , 0x00,0x00, 0x00};
	
												
uint16_t rx1_length;
uint16_t rx4_length;
uint16_t rx3_length;
uint16_t tim2_length;

uint8_t  rx1_flag = 0;
uint8_t  rx4_flag = 0;
uint8_t  rx3_flag = 0;


uint16_t k =0;              
uint8_t expose_flag = 0;    //曝光标志位
uint8_t pre_flag = 0;       //预备标志位
uint8_t Gybpre_flag =0;     //高压板是否准备
uint8_t mode_flag = 0;      //模式									 
uint8_t motor_revflag=0;    //电机反转到位
uint8_t detector_flag=0;    //探测器的连接、参数设置之后点“开始采集按钮”
uint8_t Gybpre_key=1; 			//高压板返回信号的按键
uint8_t emc_flag=0;
												
uint16_t n_total;       //拍照总张数，从上位机读取
uint16_t t_time;        //周期,帧率倒数  
uint16_t  pose_time;     //曝光时间
uint8_t  state = 0;     //用于判断进度，初始是0，pre后为1，expose结束后返回0
uint8_t stop_flag = 0;  //没用到
uint8_t sync_in=0;  //探测器的SYNCIN
uint8_t sync_out=0;  //探测器的SYNCout
uint8_t tim3flagw=0;  //
volatile  uint8_t fmqflagw=0;  //


uint8_t ring_data[RB_bufsz];
uint8_t cmd_buf[RB_bufsz];

uint16_t SynState = 0;   //同步

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

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
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_HCD_Init();
  MX_RTC_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_TIM9_Init();
  MX_UART5_Init();
  /* USER CODE BEGIN 2 */

// 使能空闲中断
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
  __HAL_UART_ENABLE_IT(&huart5, UART_IT_IDLE);
	__HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);

// 启动DMA接收
  HAL_UART_Receive_DMA(&huart1, rx1_buffer, sizeof(rx1_buffer));
	HAL_UART_Receive_DMA(&huart5, rx4_buffer, sizeof(rx4_buffer));
	HAL_UART_Receive_DMA(&huart3, rx3_buffer, sizeof(rx3_buffer));

	HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);   
	__HAL_TIM_SET_COUNTER(&htim2,0);  //清零定时器2的计数值
	
	ringbuffer rbuf;
	decodeData g_decoder;
	
	ring_buf_init(&rbuf, ring_data);
	unpack_init(&g_decoder, &rbuf, cmd_buf);

	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	
	
	t_time  = 1/20*1000 ;    //20帧用的毫秒数      
	pose_time = t_time /2 ;  //曝光时间占一半      
	
	
	
//test
/*
while (1)
  {
		//CMD_handlder_tx(TXBUF,0X01);
		
			if(rx3_flag ==1)  // 上位机下发数据
		{
			crc16_make(rx3_buffer,14);
			// 注入数据
			for (int i = 0; i < 14; i++)
			{
				ring_buf_write(&rbuf, rx3_buffer[i]);
			}

			// 处理数据
			while (ring_buf_avail(&rbuf) > 0)
			{
			unpack_fun(&g_decoder);
			}
		}
			if(motor_revflag == 1) 
			{				
				HAL_UART_Transmit(&huart3, g_decoder.buf, 14 ,0xFFFF );
	    }
			HAL_Delay(500);	
		
	}
	*/

  while (1)
  {
		
		if(rx3_flag ==1)  // 上位机下发数据
		{
			rx3_flag = 0;
		
			if(rx3_buffer[0] ==0x55 && rx3_buffer[7] == 0x02)  //给主控板
		   	{
						// 注入数据
			  	//for (int i = 0; i < sizeof(rx3_buffer); i++) 
			for (int i = 0; i < rx3_length; i++) 
					{
						ring_buf_write(&rbuf, rx3_buffer[i]);
					}

					// 处理数据
						for (int i = 0; i < rx3_length; i++) 
					{
					unpack_fun(&g_decoder);
					}
					
					//HAL_UART_Transmit(&huart3, g_decoder.buf, rx3_length ,0xFFFF );
			
			   } 
		  else if(rx3_buffer[0] ==0x55 && rx3_buffer[7] == 0x03)    //给高压板
					{
						HAL_UART_Transmit_DMA(&huart5, rx3_buffer, rx3_length);  
					}	
		  else if(rx3_buffer[0] == 0x01)  	//给电机
					{
						HAL_UART_Transmit_DMA(&huart1, rx3_buffer, rx3_length);   
					}
			else 
					{
	          CMD_handlder_tx(TXBUF0,0x05,0x01,0x00,0x00,0x0E); //报错
					}
		}
		 

  //	CMD_handlder_tx(TXBUF1,0x05,0x02,0x00,0x00,0x13);   // 查询电机位置，读定时器的计数值

		if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_RESET )  //高压板的返回有效信号，PA6
		{
			Gybpre_flag=1;
		}
		else if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_SET)
		{
			Gybpre_flag=0;
		}
			
		
		if( motor_revflag == 1 && detector_flag == 1)  //判断电机是否反转好&&探测器准备好  不持续
		{
			state = 1; //准备好了，等待按手闸二	
		  motor_revflag=0;
		  detector_flag=0;
		}
		
	  if (sync_in==1)	
			{
				if (state ==1 && Gybpre_flag ==1 &&expose_flag ==1)  //state代表一切就绪，expose代表发出pwm(手闸二)
					{ 
						state = 0;
						expose_flag =0;  //用中断函数的另一个边沿触发也可以
						HAL_TIM_Base_Start_IT(&htim4);  //启动计时器4计数，产生脉冲
						HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET); //BELL,黄色的灯
						HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_2); //板子上的蜂鸣器
					}
			}
	
			if (sync_out==1)	
			{
				if (SynState==n_total)
				{
					SynState=0;
					//HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET); //蜂鸣器
					HAL_TIM_PWM_Stop(&htim9, TIM_CHANNEL_2); //板子上的蜂鸣器
					fmqflagw=0;
					HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET); //通知高压板结束准备
          CMD_handlder_tx(TXBUF0,0x05,0x03,0x00,0x00,0x0E);
				}
			}
		
		HAL_Delay(500);   
  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 72;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
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

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)  
{  
	if(GPIO_Pin == GPIO_PIN_2)   //EMC-急停告知;PE2；上升沿
   {
		 HAL_Delay(10);
		 if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2) == GPIO_PIN_RESET)         return;
     emc_flag=1;
	 }

	 if(GPIO_Pin == GPIO_PIN_4)   //  PA4高压板的KV85
   {
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_SET)         // 上升沿
        {
				  HAL_Delay(10);
					if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET)         return;
		 
            HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET);
						HAL_TIM_Base_Stop_IT(&htim4);  	//  停止发PWM
					 
						DataFrame_Motor1 frame1 = 
						{
								.Header = 0x01,
								.Cmd = 0x05,
								.addr = 0x0123,
								.data = 0xFF00
							};
						
						DataFrame_Motor1 *p1 = &frame1;
						uint8_t data[8];
						Send_motor1(p1, data);//计算校验位CRC
						HAL_UART_Transmit_DMA(&huart1, data, sizeof(data));  //电机急停止
        }
        else                                
        {
          if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_SET)         return;
					if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_SET)         return;
					if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_SET)         return;
					if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_SET)         return;
        //  CMD_handlder_tx(TXBUF0,0X04);	//报错修好
        }
	 }	 
	
	 if(GPIO_Pin == GPIO_PIN_5)   //高压板报错，Fault。PA5。
   {
		 if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET)         // 上升沿
        {
					if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_RESET)         return;
					if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_RESET)         return;
					if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_RESET)         return;
					if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_RESET)         return;
		 
					HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET);
					HAL_TIM_Base_Stop_IT(&htim4);  	//  停止发PWM
				 
					DataFrame_Motor1 frame1 = 
					{
							.Header = 0x01,
							.Cmd = 0x05,
							.addr = 0x0123,
							.data = 0xFF00
						};
					
					DataFrame_Motor1 *p1 = &frame1;
					uint8_t data[8];
					Send_motor1(p1, data);//计算校验位CRC
					HAL_UART_Transmit_DMA(&huart1, data, sizeof(data));  //电机急停止
//				//	CMD_handlder_tx(TXBUF0,0X06);
		  	}
				
			  else                               
        {
          if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET)         return;
					if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET)         return;
					if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET)         return;
					if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET)         return;
         // CMD_handlder_tx(TXBUF0,0X07);	//报错修好
        }	
	 }
	 
	 if(GPIO_Pin == GPIO_PIN_6)  //PE6,手闸二；下降沿
   {
		 if(state == 1 && Gybpre_flag ==1)
		{
			if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_6) == GPIO_PIN_SET)         return;
			if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_6) == GPIO_PIN_SET)         return;
			if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_6) == GPIO_PIN_SET)         return;
			if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_6) == GPIO_PIN_SET)         return;
	  	expose_flag =1;
		 }
	 }
	 
	 if(GPIO_Pin == GPIO_PIN_13)  //SYNCOUT探测器的采集请求信号;PE13
   {  
		 if(sync_out==1 && state == 1 && Gybpre_flag ==1)
		 {
			 expose_flag =1;
			}
	 }
	 
	 	if(GPIO_Pin == GPIO_PIN_14)  //探测器返回信号FPD;PE14
   {
		if(tim3flagw==0)
		{
			if(sync_in==1 && Gybpre_flag ==1 )
			{
		   	__HAL_TIM_SET_COUNTER(&htim3,0);  //清零定时器3的计数值
			 HAL_TIM_Base_Start_IT(&htim3);
			 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);   //拉高曝光引脚COM ,PC4
			 tim3flagw=1;
		  }

			if(sync_out==1 && state ==1 && Gybpre_flag ==1 &&expose_flag ==1)
			{
				if(fmqflagw==0)
				{
					HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_2); //板子上的蜂鸣器
					fmqflagw=1;
				}
				if(SynState < n_total)
						{
							SynState ++;
						  expose_flag =0; 
							__HAL_TIM_SET_COUNTER(&htim3,0);  //清零定时器3的计数值
						 HAL_TIM_Base_Start_IT(&htim3);
						 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);   //拉高曝光引脚COM ,PC4
						tim3flagw=1;
						}
			}
			
		}
	 }
	 
 }

 void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)  //定时器中断回调函数
{
		if (htim->Instance == TIM3)  
    {
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);   //拉低曝光引脚COM ,PC4
        HAL_TIM_Base_Stop_IT(&htim3);  	//  停止定时器（避免计数干扰）
				tim3flagw=0;
    }
		
		if (htim->Instance == TIM4)  
    {
			if(SynState < 2 * n_total)
			{
				HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_15);    //SYNC_InP,启动同步
				SynState ++;
			}
			else
			{
				SynState=0;
		    HAL_TIM_Base_Stop_IT(&htim4);  	//  停止定时器（避免计数干扰）
				HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET); //bell黄灯
				HAL_TIM_PWM_Stop(&htim9, TIM_CHANNEL_2); //板子上的蜂鸣器
				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET); //IO口通知高压板结束准备
        CMD_handlder_tx(TXBUF0,0x05,0x03,0x00,0x00,0x0E);
			}
    }
}
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

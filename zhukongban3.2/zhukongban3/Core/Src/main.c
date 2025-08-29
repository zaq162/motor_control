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
#include "can.h"
#include "dma.h"
#include "i2c.h"
#include "rtc.h"
#include "tim.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hardware.h"
#include "stm32f4xx.h" 
uint8_t rx1_buffer[RX_BUFFER_SIZE];   //接电机，串口1 485
uint8_t rx4_buffer[RX_BUFFER_SIZE];   //接高压板，串口4  232
uint8_t rx3_buffer[RX_BUFFER_SIZE];   //接上位机，串口3 422
uint8_t tim2_buffer[5];
uint8_t fault_buffer0[11]= {0x5A, 0x01, 0x08, 0x66, 0x66,
                       0x00, 0x00, 0x01, 0x00, 0x00, 0x55};
uint8_t fault_buffer1[11]= {0x5A, 0x01, 0x08, 0x88, 0x88,
                       0x00, 0x00, 0x01, 0x00, 0x00, 0x55};
uint8_t fault_buffer2[11]= {0x5A, 0x01, 0x08, 0x99, 0x99,
                       0x00, 0x00, 0x01, 0x00, 0x00, 0x55};
											 
uint16_t rx1_length;
uint16_t rx4_length;
uint16_t rx3_length;
uint16_t tim2_length;

uint8_t  rx1_flag = 0;
uint8_t  rx4_flag = 0;
uint8_t  rx3_flag = 0;

uint8_t motor_flag = 0;     //如果等于0则对应信号转高压板
uint8_t com_flag = 0;       //如果等于1则对应信号转高压板

uint16_t k =0;              
uint8_t expose_flag = 0;    //曝光标志位
uint8_t pre_flag = 0;       //预备标志位
uint8_t Gybpre_flag =0;     //高压板是否准备
uint8_t Syn_flag =0;        //同步标志位,没用到
uint8_t over_flag = 0;      //结束标志位，没用到
uint8_t fault_flag=0;       //高压板报错
uint8_t motor_revflag=0;    //电机反转到位
uint8_t detector_flag=0;    //探测器的连接、参数设置之后点“开始采集按钮”
uint8_t Gybpre_key=1; 			//高压板返回信号的按键

uint16_t n_total;       //拍照总张数，从上位机读取
uint16_t t_time;        //周期,帧率倒数  
uint8_t  pose_time;     //曝光时间
uint8_t  state = 0;     //用于判断进度，初始是0，pre后为1，expose结束后返回0
uint8_t stop_flag = 0;  //没用到
uint8_t sync_in=0;  //探测器的SYNCIN
uint8_t sync_out=0;  //探测器的SYNCout

uint32_t pos = 0; // 编码器计数值
uint8_t diretion=0;//编码器方向,逆时针是01，顺时针是00

unsigned char SynState = 0;   //同步
unsigned char ExpState = 0;   //曝光
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
  MX_CAN1_Init();
  MX_CAN2_Init();
  MX_I2C3_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_HCD_Init();
  MX_RTC_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_UART4_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */

//	HAL_UART_Receive_IT(&huart1,rx_buffer ,sizeof(rx_buffer));  //中断接收

// 使能空闲中断
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
  __HAL_UART_ENABLE_IT(&huart4, UART_IT_IDLE);
	__HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);

//  // 启动DMA接收
  HAL_UART_Receive_DMA(&huart1, rx1_buffer, sizeof(rx1_buffer));
	HAL_UART_Receive_DMA(&huart4, rx4_buffer, sizeof(rx4_buffer));
	HAL_UART_Receive_DMA(&huart3, rx3_buffer, sizeof(rx3_buffer));



	HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);   // 一次启动所有通道
	__HAL_TIM_SET_COUNTER(&htim2,0);  //清零定时器2的计数值
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	
	
	t_time  = 1/20*1000 ;    //20帧用的毫秒数      
	pose_time = t_time /2 ;  //曝光时间占一半      
	
  while (1)
  {
		
		if(rx3_flag ==1)  // 上位机下发数据
		{
			rx3_flag = 0;
			if(rx3_buffer[0] == 0x5A)  //除电机外的数据
			{
				GybMessage message_422 = 
				{
					.Header = rx3_buffer[0], //数据帧头
					.Addr = rx3_buffer[1],  // 数据地址
					.Len = rx3_buffer[2],   // 数据长度
					.Byte0 = rx3_buffer[3], // 指令
					.Byte1 = rx3_buffer[4], // 特殊位索引1
					.Byte2 = rx3_buffer[5], // 特殊位索引 2
					.Byte3 = rx3_buffer[6], // 特殊位索引 3
					.Byte4 = rx3_buffer[7], // 数据字节 1
					.Byte5 = rx3_buffer[8], // 数据字节 2
					.Byte6 = rx3_buffer[9], // 数据字节 3
					.Byte7 = rx3_buffer[10] // 数据字节 4
				};
				
				if(message_422.Byte0 == 0x21 && message_422.Byte4 == 0x01)   //上位机手闸一
				{
					pre_flag = 1;
				}
				else if(message_422.Byte0 == 0x22 && message_422.Byte4 == 0x01)  //上位机手闸二
				{
					expose_flag = 1;
				}
				else if(message_422.Byte0 == 0x55 && message_422.Byte1 == 0x66)  //松开电机刹车线
				{
					HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);  //低电平有效
				}
				else if(message_422.Byte0 == 0x55 && message_422.Byte1 == 0x55)  //电机反转
				{
					motor_revflag = 1;
				}
				else if(message_422.Byte0 == 0x66 && message_422.Byte1 == 0x01 && message_422.Byte2 == 0x01)  //在探测器的syncin
				{
					sync_in = 1;
				}
				else if(message_422.Byte0 == 0x66 && message_422.Byte1 == 0x02 && message_422.Byte2 == 0x02)  //在探测器的syncout
				{
					sync_out = 1;
				}
				else if(message_422.Byte0 == 0x77 && message_422.Byte1 == 0x77)  //在探测器的连接、参数设置之后点“开始采集按钮”
				{
					detector_flag = 1;
				}
				else if(message_422.Byte0 == 0x23 && message_422.Byte4 == 0x01)   //上位机停止
				{
					stop_flag = 1;
				}
				else if(message_422.Byte0 == 0x27)         //拍照总张数
				{
					n_total = message_422.Byte4*256 + message_422.Byte5;
				}
				else if(message_422.Byte0 == 0x15)         //脉冲曝光周期 T
				{
					t_time = message_422.Byte3*256 + message_422.Byte4;  //t_time它的单位是0.1ms,
					
          __HAL_TIM_SET_AUTORELOAD(&htim4, t_time/2-1);  // 修改 TIM4 的自动重装载值（ARR）
         // __HAL_TIM_SET_PRESCALER(&htim2, new_psc);   // 修改 TIM4 的和预分频（PSC）
          HAL_TIM_GenerateEvent(&htim2, TIM_EVENTSOURCE_UPDATE);  //立即生效
				}
				else if(message_422.Byte0 == 0x16)         //曝光时间 T1
				{
					pose_time = message_422.Byte3*256 + message_422.Byte4;
				 __HAL_TIM_SET_AUTORELOAD(&htim3, pose_time-1);  // 修改 TIM4 的自动重装载值（ARR）
					 HAL_TIM_GenerateEvent(&htim2, TIM_EVENTSOURCE_UPDATE); 
				}
				else
				{
					com_flag = 1;   
					HAL_UART_Transmit_DMA(&huart4, rx3_buffer, rx3_length);   //转发给高压板
				}	
			}
		
			else if(rx3_buffer[0] == 0x01)  	//增加一个判断标志，判断是否是发送给电机的数据
			{
				motor_flag =1;
			 
				HAL_UART_Transmit_DMA(&huart1, rx3_buffer, rx3_length);   //转发给电机
			}
		}
		
	 
		// 查询电机位置，但是读的是定时器的计数和电机无关
    pos = __HAL_TIM_GET_COUNTER(&htim2);
		diretion=__HAL_TIM_IS_TIM_COUNTING_DOWN(&htim2);
		tim2_buffer[0] =diretion;
		tim2_buffer[1] = (uint8_t)(pos & 0xFF);
    tim2_buffer[2] = (uint8_t)((pos >> 8) & 0xFF);
    tim2_buffer[3] = (uint8_t)((pos >> 16) & 0xFF);
    tim2_buffer[4] = (uint8_t)((pos >> 24) & 0xFF);
    tim2_length = 5;
		HAL_UART_Transmit(&huart3, tim2_buffer, tim2_length ,0xFFFF );
	
		
		if (pre_flag == 1)  //判断是否来了准备信号
		{
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET); //IO口通知高压板准备
			pre_flag = 0;	
			
	  }
		
		if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_RESET && Gybpre_key==1 )  //高压板的返回有效信号，PA5,一次连续的低电平只Gybpre_flag=1，一次
		{
			Gybpre_key=0;
		}
		else if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_SET)
		{
	  	Gybpre_key=1;
		}
			
		
		if( Gybpre_flag == 1 && motor_revflag == 1 && detector_flag == 1)  //判断电机是否反转好&&高压板是否准备好&&探测器准备好
		{
			state = 1; //状态机	
			Gybpre_flag=0;
			motor_revflag=0;
			detector_flag=0;
		}
		
	  if (sync_in==1)	
			{
				sync_in=0;
				if (state ==1 && expose_flag ==1)  //state代表一切就绪，expose代表发出pwm(手闸二)
					{ 
						state = 0;
						expose_flag =0;
						HAL_TIM_Base_Start_IT(&htim4);  //启动计时器4计数，产生PWM
						HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET); //蜂鸣器
					}
			}
	
		
		HAL_Delay(500);   

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
//外部中断回调函数，其实依然可以使用查询方式读取信号状态,目前中断过多，可以删除
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)  
{  
	 if(GPIO_Pin == GPIO_PIN_2)   //EMC-急停告知;PE2
   {
		HAL_UART_Transmit(&huart3, fault_buffer1, 11 ,0xFFFF );
	 }

	 if(GPIO_Pin == GPIO_PIN_4)   //  PA4高压板的KV85
   {
		HAL_UART_Transmit(&huart3, fault_buffer0, 11 ,0xFFFF );
	 }	 
	
	 if(GPIO_Pin == GPIO_PIN_5)   //高压板报错，Fault。PA5
   {
		fault_flag = 1;
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
				
			HAL_UART_Transmit(&huart3, fault_buffer2, 11 ,0xFFFF );	
	 }
	 
	 if(GPIO_Pin == GPIO_PIN_6)  //PE6,手闸二
   {
		expose_flag =1;
	 }
	 
	 if(GPIO_Pin == GPIO_PIN_13)  //SYNCOUT探测器的采集请求信号;PE13
   {  
		 if(sync_out==1)
		 {
			 if(SynState < n_total)
				{
					HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);   //拉高曝光引脚COM ,PC4
					HAL_TIM_Base_Start_IT(&htim3);
					SynState ++;
				}
				else
				{
					SynState=0;
					HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET); //蜂鸣器
					HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET); //IO口通知高压板结束准备
				}
			}
	 }
	 
	 	if(GPIO_Pin == GPIO_PIN_14)  //探测器返回信号FPD;PE14
   {
	 // __HAL_TIM_SET_COUNTER(&htim3,0);  //清零定时器3的计数值
		 HAL_TIM_Base_Start_IT(&htim3);
		 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_SET);   //拉高曝光引脚COM ,PC4
	 }
	 
 }

 void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)  //定时器中断回调函数
{
		if (htim->Instance == TIM3)  
    {
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);   //拉低曝光引脚COM ,PC4
        HAL_TIM_Base_Stop_IT(&htim3);  	//  停止定时器（避免计数干扰）
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
				HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET); //蜂鸣器
				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET); //IO口通知高压板结束准备
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

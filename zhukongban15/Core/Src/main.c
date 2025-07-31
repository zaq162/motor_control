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
uint8_t rx1_buffer[RX_BUFFER_SIZE];   //接电机，串口1
uint8_t rx4_buffer[RX_BUFFER_SIZE];   //接高压板，串口4
uint8_t rx3_buffer[RX_BUFFER_SIZE];   //接上位机，串口3

uint16_t rx1_length;
uint16_t rx4_length;
uint16_t rx3_length;

uint8_t  rx1_flag = 0;
uint8_t  rx4_flag = 0;
uint8_t  rx3_flag = 0;

uint8_t motor_flag = 0;     //如果等于0则对应信号转高压板
uint8_t com_flag = 0;       //如果等于1则对应信号转高压板

uint16_t k =0;              
uint8_t expose_flag = 0;    //曝光标志位
uint8_t pre_flag = 0;       //预备标志位
uint8_t Gybpre_flag =0;     //高压板是否准备
uint8_t Syn_flag =0;        //同步标志位
uint8_t over_flag = 0;      //结束标志位

uint16_t n_total;       //拍照总张数，从上位机读取
uint16_t t_time;        //周期,帧率倒数  
uint8_t  pose_time;     //曝光时间
uint8_t  state = 0;     //用于判断进度，初始是0，pre后为1，expose结束后返回0
uint8_t stop_flag = 0;

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
  /* USER CODE BEGIN 2 */
//中断接收
//	HAL_UART_Receive_IT(&huart1,rx_buffer ,sizeof(rx_buffer));
// 使能空闲中断
  __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);
  __HAL_UART_ENABLE_IT(&huart4, UART_IT_IDLE);
	__HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);

//  // 启动DMA接收
  HAL_UART_Receive_DMA(&huart1, rx1_buffer, sizeof(rx1_buffer));
	HAL_UART_Receive_DMA(&huart4, rx4_buffer, sizeof(rx4_buffer));
	HAL_UART_Receive_DMA(&huart3, rx3_buffer, sizeof(rx3_buffer));

				//使能TIM2中断
	HAL_TIM_Base_Start_IT(&htim2);    //	
	HAL_TIM_Base_Start_IT(&htim3);    //	
	HAL_TIM_Base_Stop(&htim2);     //暂停计数 
	HAL_TIM_Base_Stop(&htim3);     //暂停计数 
  //HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_3);  //启动PWM计数
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	
	
	t_time  = 1/20*1000 ;    //20帧用的毫秒数      //这两个数据需要通过上位机下发指令读取
	pose_time = t_time /2 ;  //曝光时间占一半      //可修改，上位机可
	
  while (1)
  {
		//定时器定时查询，目前是延时1秒查询，不用延时较好
		
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
				else if(message_422.Byte0 == 0x23 && message_422.Byte4 == 0x01)   //上位机停止
				{
					stop_flag = 1;
				}
				
				else if(message_422.Byte0 == 0x15)         //曝光总时间
				{
					t_time = message_422.Byte3*256 + message_422.Byte4;
				}
				else if(message_422.Byte0 == 0x16)         //脉冲曝光时间
				{
					pose_time = message_422.Byte3*256 + message_422.Byte4;
				}
				else
				{
					com_flag = 1;   
					HAL_UART_Transmit_DMA(&huart4, rx3_buffer, rx3_length);   //转发给高压板
				}	
			}
			//增加一个判断标志，判断是否是发送给电机的数据，比如头是否是01
			else if(rx3_buffer[0] == 0x01)
			{
				motor_flag =1;
				HAL_UART_Transmit_DMA(&huart1, rx3_buffer, rx3_length);   //转发给电机
			}
		}
		
		
		//判断是否来了准备信号，下面的过程较为复杂但是单一，建议使用状态机
		if (pre_flag == 1)
		{
				//发动到高压板
			HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_SET); //IO口通知高压板准备
			pre_flag = 0;	
			state = 1; //状态机
	  }
			
		//查询电机速度，
		
				HAL_Delay(25);  
			DataFrame_Motor1 frame1 = 
			{
					.Header = 0x01,
					.Cmd = 0x03,
					.addr = 0x4025,
					.data = 0x0001
				};
			
			DataFrame_Motor1 *p1 = &frame1;
			uint8_t data[8];
			Send_motor1(p1, data);//计算校验位CRC
			HAL_UART_Transmit_DMA(&huart1, data, sizeof(data));  //通过串口1发送指令查询电机速度
		
							
//查询电机位置，
						HAL_Delay(25);  
			DataFrame_Motor1 frame2 = 
			{
					.Header = 0x01,
					.Cmd = 0x03,
					.addr = 0x4204,
					.data = 0x0002
				};
			
			DataFrame_Motor1 *p2 = &frame2;
			uint8_t data1[8];
			Send_motor1(p2, data1);//计算校验位CRC
			HAL_UART_Transmit_DMA(&huart1, data1, sizeof(data1));  //通过串口1发送指令查询电机位置
			
				
		//判断电机是否转好并且高压板是否准备好同时判断是否手闸二打开
		if(state ==1 && Gybpre_flag == 1)
		{
		
				//后面可以通过读取电机速度是否为零进行判断。
				
			if (expose_flag ==1)  //手闸2打开，开始曝光
			{
				//启动计时器2计数，产生同步信号 
				HAL_TIM_Base_Start(&htim2);
	  	}
		}
		
		if(over_flag ==1)  //通过判断曝光次数或者电机运动停止
		{
			//通知高压板和上位机曝光结束
		}
			HAL_Delay(25);   

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
	if(GPIO_Pin == GPIO_PIN_14)  //探测器返回信号FPD
   {
		Syn_flag = 1;
		//启动定时器3
		 // 1. 设置计数器为 0（写入影子寄存器）
		 HAL_TIM_Base_Start(&htim3);
		 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);   //拉高曝光引脚COM ,PC4
	 }
	 
	 //增加一个判断
	 if(GPIO_Pin == GPIO_PIN_4)   //手闸一
   {
		pre_flag = 1;
	 }	 
	 
	 if(GPIO_Pin == GPIO_PIN_2)   //手闸二
   {
		expose_flag = 1;
	 }
	 
	 if(GPIO_Pin == GPIO_PIN_6)  //高压板回准备信号
   {
		Gybpre_flag = 1;
	 }
	 
 }

 void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)  //定时器中断回调函数
{
// static unsigned char ledState = 0;
    // 用 Instance 判断更通用，兼容未来多定时器扩展
    if (htim->Instance == TIM2)  
    {
			if(SynState < 2 * n_total)
			{
				HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_15);    //SYNC_InP,启动同步
				SynState ++;
			}
			else
			{
				over_flag = 1;
			}
			
    }
		if (htim->Instance == TIM3)  
    {
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, GPIO_PIN_RESET);   //拉低曝光引脚COM ,PC4
		//  停止定时器（避免计数干扰）
        HAL_TIM_Base_Stop(htim);  
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


#include "main.h"
#include "tim.h"
#include "gpio.h"
void SystemClock_Config(void);
////第一种：while 轮询，使用计数器i
volatile uint32_t i=0;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{if (htim->Instance == TIM4) //回调函数是定时器发生中断时自动调用
    {
i++;
		}
}
void delay_ms(uint32_t ms);
{ HAL_TIM_Base_Start_IT(&htim4);//开启定时器以及溢出中断
    uint32_t start = i;
    while ((i - start) < ms) {
			__NOP(); // 空指令避免编译器优化
    }
		HAL_TIM_Base_Stop_IT(&htim4);
}
//第二种：使用标志位，不使用计数器
volatile uint8_t timer_flag = 1;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{if (htim->Instance == TIM4) //回调函数是定时器发生中断时自动调用
    {
			 timer_flag = 1;
       HAL_TIM_Base_Stop_IT(&htim4);
		}
}

void delay_us(uint32_t us1, uint16_t us2)
{ 
	  MX_TIM4_Init(us1-1,us2-1);
	HAL_TIM_Base_Start_IT(&htim4);//开启定时器以及溢出中断
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_TIM4_Init(65535,83);
  while (1)
  {
		if (timer_flag)
        {
        timer_flag = 0;
				HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_10);
				delay_us(0xffffffff, 0xff);
				}
  }
}



int main(void)
{
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_TIM4_Init();

  while (1)
  {
		HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_10);
	delay_ms(5);
  }
}





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
  RCC_OscInitStruct.PLL.PLLN = 168;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void delay_ms(uint32_t ms)
{ HAL_TIM_Base_Start_IT(&htim4);//开启定时器以及溢出中断
//使用减法可以自动处理溢出，这是无符号整数运算的特性。
    uint32_t start = i;
	  
    while ((i - start) < ms) {
     //计数等待
			__NOP(); // 空指令避免编译器优化
    }
		HAL_TIM_Base_Stop_IT(&htim4);

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

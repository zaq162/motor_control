//在stm32f4xx_it.c  定时器中断进入函数

/*void TIM4_IRQHandler(void)
{
  TimerCount++;
  HAL_TIM_IRQHandler(&htim4);
}  */



/* 张工_定时器中断延时。较大延时不占用mpu ------------------------------------------------------------------*/

#include "zhangTimerDelay.h"
#include "stm32f4xx_hal.h"
#include "tim.h"
#include "gpio.h"

extern uint32_t TimerCount;
extern uint32_t TimCouTarget;
extern uint8_t timeflag;//置0可以使用timdelay

void TimDelay_ms(uint32_t ms)
{ HAL_TIM_Base_Start_IT(&htim4);//开启定时器以及溢出中断
	      if(timeflag==0)
					{
				  TimCouTarget =TimerCount+ms;
					timeflag=1;
			  	}
				
				if(TimerCount>=TimCouTarget )
				{	timeflag=0;
					 HAL_TIM_Base_Stop_IT(&htim4);//关定时器以及溢出中断
					TimerCount=0;//计数器清零
					//需要延时后执行的代码
					HAL_GPIO_TogglePin(GPIOF, GPIO_PIN_9);
				}

}





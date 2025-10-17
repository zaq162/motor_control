#include "cmdhandle.h"
#include "stdint.h"
#include <stdlib.h>   
#include "unpack.h"
#include "crc.h"
#include "dma.h"
#include "usart.h"
#include "rtc.h"
#include "tim.h"
#include "shared_var.h"

extern uint8_t rx3_buffer[RX_BUFFER_SIZE];
extern uint16_t rx3_length;
extern uint8_t motor_revflag;
extern uint8_t expose_flag;
extern uint8_t sync_in; 
extern uint8_t sync_out;
extern uint16_t n_total; 
extern uint16_t t_time;
extern uint16_t  pose_time;  
extern uint8_t mode_flag ;  
extern uint8_t detector_flag;
extern ringbuffer  rbuf;
extern decodeData  g_decoder;

void CMD_handlder_fun(uint8_t* buf)
{
	CMD_Pack *str;
	str = (CMD_Pack *)buf;
	if (str->src_ID == Host_computer_ID) //
	{
		if (str->dst_ID == Control_board_ID)
		{
				
					if (str->CMD_set == 0x01)
					{	
						switch (str->CMD_ID)
						{
						case 1:
							//GetDeviceState(buf);
							break;
						case 2:
						//	GetFirmware(buf);
							break;
						case 3:
						//	GetHardwareCode(buf);
							break;
						case 4:
						//	WriteHardwareCode(buf);
							break;
						case 5:
						 if (SHARED->magic == 0xDEADBEEF)
							{
									uint32_t v = SHARED->flag;   /* 应为 0x55 */
									SHARED->flag = v + 0x11;        /* 回写 0x56 */
							}
							break;
						case 6:
			       __disable_irq();
						HAL_NVIC_SystemReset(); //系统复位
							break;
						case 7:
					//		UpdateSuccess(buf);
							break;
						case 8:
					//		RebootReq(buf);
							break;
						case 9:
						//	FlaskOpera(buf);
							break;
						case 10:
						//	ModifyBlueName(buf);
							break;
						case 11:
						//	cam_config_fun(buf);
							break;
						case 255://baiws 测试0xFF
						//	printf("baiws 测试0xFF\n");
						//	Test_info(buf);
							break;
						default:
							break;
						}
					}

					if (str->CMD_set == 0x02)  //电机
					{
						switch (str->CMD_ID)
						{
						case 0x01:
							HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
						
							break;
						case 2:
					  	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET); 
							break;
						case 3:
							motor_revflag = 1;
							break;
						
						default:
							break;
						}
					}

					if (str->CMD_set ==0x03)   //HV
					{
						switch (str->CMD_ID)
						{
						case 1:
							HAL_GPIO_WritePin(GPIOA,GPIO_PIN_7,GPIO_PIN_RESET);
							break;
						case 2:
							expose_flag = 1;
							break;
						case 3:
							sync_in = 0;
							sync_out = 0;
							HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET); 
							break;
						default:
							break;
						}
					}
		     	
					if (str->CMD_set ==0x04)   //detector_flag
					{
						switch (str->CMD_ID)
						{
						case 1:
							if (buf[12]==0x01)  sync_in = 1;
						  if (buf[12]==0x02)  sync_out = 1;
							break;
						case 2:
							n_total = buf[13]* 256 + buf[12];
							break;
						case 3:
							t_time = buf[13]*  256 + buf[12];  //t_time它的单位是0.1ms,

							__HAL_TIM_SET_AUTORELOAD(&htim4, t_time / 2 - 1);  // 修改 TIM4 的自动重装载值（ARR）
							// __HAL_TIM_SET_PRESCALER(&htim2, new_psc);   // 修改 TIM4 的和预分频（PSC）
							HAL_TIM_GenerateEvent(&htim2, TIM_EVENTSOURCE_UPDATE);  //立即生效
							break;
						case 4:
							pose_time =buf[13]*  256 + buf[12];
							__HAL_TIM_SET_AUTORELOAD(&htim3, pose_time - 1);  // 修改 TIM4 的自动重装载值（ARR）
							HAL_TIM_GenerateEvent(&htim2, TIM_EVENTSOURCE_UPDATE);				
							break;
						case 5:
							if ( buf[12] == 0x11)  mode_flag = 1;
							if ( buf[12] == 0x22)  mode_flag = 2;
							if ( buf[12] == 0x33)  mode_flag = 3;
							if ( buf[12] == 0x44)  mode_flag = 4;
							break;
						case 6:
					  detector_flag = 1;
					  break;
						
					
						
						default:
						break;
						}
					}
   	}
		
		
}
	

}


void CMD_handlder_tx(uint8_t* buf,uint8_t cmd_id)
{
	CMD_Pack *str;
	str = (CMD_Pack *)buf;
	str->CMD_ID=cmd_id;
	generate_head_crc(buf);
  crc16_make(buf,14);
	HAL_UART_Transmit(&huart3, buf, 14 ,0xFFFF );
}
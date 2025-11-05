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
extern uint8_t expose_flag ;
extern uint8_t TXBUF0[14];
extern uint8_t emc_flag;
extern uint8_t Gybpre_flag;

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
							if (str->ack ==0x01)   
					{
						CMD_handlder_tx(TXBUF0,0x01,0x01,0x00,0x00);
					}
							break;
						case 2:
						if (str->ack ==0x01)   
					{
						CMD_handlder_tx(TXBUF0,0x01,0x02,0x00,0x00);
					}
							break;
						case 3:
						if (str->ack ==0x01)   
					{
						CMD_handlder_tx(TXBUF0,0x01,0x03,0x00,0x00);
					}
							break;
						case 4:
						//	WriteHardwareCode(buf);
							break;
						case 5:
						 if (SHARED->magic == 0xDEADBEEF)
							{
									uint32_t v = SHARED->flag;      /* 应为 0x55 */
									SHARED->flag = v + 0x11;        /* 回写 0x56 */
							}
							break;
						case 6:
			       
							break;
						case 7:
					//		UpdateSuccess(buf);
							break;
						case 8:
					__disable_irq();
						HAL_NVIC_SystemReset(); //系统复位
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
						case 4:
							if (str->ack ==0x01)   
					{
							if (expose_flag ==1)   	CMD_handlder_tx(TXBUF0,0x03,0x04,0x00,0x01);
					    if (expose_flag ==0)   	CMD_handlder_tx(TXBUF0,0x03,0x04,0x00,0x00);
					}
							break;
						case 5:
							if (str->ack ==0x01)   
					{
							if (emc_flag ==1)   	CMD_handlder_tx(TXBUF0,0x03,0x05,0x00,0x01);
					    if (emc_flag ==0)   	CMD_handlder_tx(TXBUF0,0x03,0x05,0x00,0x00);
					}
							if (str->ack ==0x00)   
					{
							emc_flag=0;
					}
							break;
					case 6:
							if (str->ack ==0x01)   
					{
							if (Gybpre_flag ==1)   	CMD_handlder_tx(TXBUF0,0x03,0x06,0x00,0x01);
					    if (Gybpre_flag ==0)   	CMD_handlder_tx(TXBUF0,0x03,0x06,0x00,0x00);
					}
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


void CMD_handlder_tx(uint8_t* buf,uint8_t cmd_set,uint8_t cmd_id,uint8_t ack,uint8_t res)
{
	CMD_Pack *str;
	str = (CMD_Pack *)buf;
	uint16_t len=0;
	
	len=sizeof (buf);
	generate_head_crc(buf);
	str->CMD_set=cmd_set;
	str->CMD_ID=cmd_id;
	str->ack=ack;
	str->res=res;
	if(cmd_set==0x05 && cmd_id==0x02)
	{
	  uint32_t pos = __HAL_TIM_GET_COUNTER(&htim2);
		uint8_t  diretion=__HAL_TIM_IS_TIM_COUNTING_DOWN(&htim2);//编码器方向,逆时针是01，顺时针是00
		buf[12] =diretion;
		buf[13] = (uint8_t)(pos & 0xFF);
    buf[14] = (uint8_t)((pos >> 8) & 0xFF);
    buf[15] = (uint8_t)((pos >> 16) & 0xFF);
    buf[16] = (uint8_t)((pos >> 24) & 0xFF);
	}
	
		if(cmd_set==0x01 && cmd_id==0x01)
	{
	 
		buf[12] = 0x01;
		buf[13] = 0x00;
    buf[14] = 0x00;
    buf[15] = 0x00;
 
	}
	
		if(cmd_set==0x01 && cmd_id==0x02)
	{
	 
		buf[12] = 0x01;
		buf[13] = 0x00;
    buf[14] = 0x00;
    buf[15] = 0x00;
 
	}
	
		if(cmd_set==0x01 && cmd_id==0x03)
	{
	 
		buf[12] = 0x01;
		buf[13] = 0x00;
    buf[14] = 0x00;
    buf[15] = 0x00;
 
	}
	
	
  crc16_make(buf,len);
	HAL_UART_Transmit(&huart3, buf, len ,0xFFFF );
}
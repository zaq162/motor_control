#include "cmdhandle.h"
#include "stdint.h"
#include <stdlib.h>   
#include "unpack.h"
#include "crc.h"
#include "dma.h"
#include "usart.h"
#include "shared_var.h"
extern uint8_t rx3_buffer[RX_BUFFER_SIZE];
extern uint16_t rx3_length;

extern uint8_t flashwr_flag;
extern uint8_t flasherase_flag;
extern uint8_t	flashover_flag;
//extern uint32_t  section0;
extern uint32_t APP_address;

extern ringbuffer  rbuf;
extern decodeData  g_decoder;
extern Share_t s;
extern Share_t s1;
extern Share_t s2;
extern Share_t s3;
extern uint8_t TXBUF0[14];

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
								SHARE_Read(&s, SHARE_ADDR);

						 if (s.magic == SHARE_MAGIC)
							{
									SHARE_Erase( 3 );
									SHARE_Read(&s, SHARE_ADDR);
									if(s.magic==0xFFFFFFFF)  
									{
										SHARE_Write(&s2,3);
									}
							}
							break;
						case 6:
								flashwr_flag=1;
							break;
						case 7:
							
						
							break;
				  	case 9:
					flashover_flag=1;
				
							break;
						case 0x0A:
						flasherase_flag=1;
				
							break;
							case 0x0B:
						flashover_flag=1;
				
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
	
			if(cmd_set==0x01 && cmd_id==0x01)
	{	
		SHARE_Read(&s, SHARE_ADDR);
	 
		buf[12] = (uint8_t)(s.state & 0xFF);
    buf[13] = (uint8_t)((s.state >> 8) & 0xFF);
    buf[14] = (uint8_t)((s.state >> 16) & 0xFF);
    buf[15] = (uint8_t)((s.state >> 24) & 0xFF);
		
	}
	
		if(cmd_set==0x01 && cmd_id==0x02)
	{
	 	SHARE_Read(&s, SHARE_ADDR);
	buf[12] = (uint8_t)(s.hardwarecode & 0xFF);
    buf[13] = (uint8_t)((s.hardwarecode >> 8) & 0xFF);
    buf[14] = (uint8_t)((s.hardwarecode >> 16) & 0xFF);
    buf[15] = (uint8_t)((s.hardwarecode >> 24) & 0xFF);
 
	}
	
		if(cmd_set==0x01 && cmd_id==0x03)
	{
	 	SHARE_Read(&s, SHARE_ADDR);
		buf[12] = (uint8_t)(s.softwarenumb & 0xFF);
    buf[13] = (uint8_t)((s.softwarenumb >> 8) & 0xFF);
    buf[14] = (uint8_t)((s.softwarenumb >> 16) & 0xFF);
    buf[15] = (uint8_t)((s.softwarenumb >> 24) & 0xFF);
 
	}
	
	
  crc16_make(buf,len);
	HAL_UART_Transmit(&huart3, buf, len ,0xFFFF );
}
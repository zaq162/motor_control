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
extern uint32_t  section;
extern uint32_t APP_address;

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
					
						case 6:
							//EnterUpadataPro(buf);
							break;
						case 7:
							 if (SHARED->magic == 0xDEADBEEF)
								{
										uint32_t v = SHARED->flag;  
										SHARED->flag = v - 0x11;       
								}
								 //APP_address=APP_ADDR;
							break;
				  	case 9:
						flashwr_flag=1;
						APP_address=buf[15]* 256* 256* 256  +buf[14]* 256* 256 + buf[13]* 256 + buf[12];
							break;
						case 0x0A:
						flasherase_flag=1;
						section=buf[12];
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
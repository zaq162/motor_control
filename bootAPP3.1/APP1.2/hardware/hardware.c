
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

#include "dma.h"

#include "rtc.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"
#include "hardware.h"


void sys_expose(uint16_t k, uint16_t t_time, uint8_t  pose_time, uint8_t state)
{
	if(k % (t_time/2) ==0)
		{
			HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_15);    //SYNC_InP,????
		}
		
		if(state ==0 && k % t_time ==1)
		{
			 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4 ,GPIO_PIN_SET);  // ????
			 state =1;
		}
		else if(state ==1 && k % pose_time ==1)
		{
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4 , GPIO_PIN_RESET);  //????
			state =0;
		}
}

void Send_motor1(DataFrame_Motor1 *p1, uint8_t *data) {
    uint8_t addrLow = (uint8_t)(p1->addr & 0xFF);
    uint8_t addrHigh = (uint8_t)(p1->addr >> 8);
    uint8_t dataLow = (uint8_t)(p1->data & 0xFF);
    uint8_t dataHigh = (uint8_t)(p1->data >> 8);

    uint8_t dataToCalculateCRC[] = {
        p1->Header, p1->Cmd, addrHigh, addrLow, dataHigh, dataLow
    };
    
    size_t dataLen = sizeof(dataToCalculateCRC) / sizeof(dataToCalculateCRC[0]);
    uint16_t crc = CalculateCRC16(dataToCalculateCRC, dataLen);
    p1->CRC_LOW = (uint8_t)(crc & 0xFF);
    p1->CRC_HIGH = (uint8_t)(crc >> 8);

    data[0] = p1->Header;
    data[1] = p1->Cmd;
    data[2] = addrHigh;
    data[3] = addrLow;
    data[4] = dataHigh;
    data[5] = dataLow;
    data[6] = p1->CRC_LOW;
    data[7] = p1->CRC_HIGH;
}

void Send_motor2(DataFrame_Motor2 *p2, uint8_t *data)
{
	
		// ? ushort ??? addr ? data ????? uint8_t ???? 
	uint8_t addrLow = (uint8_t)(p2->addr & 0xFF);
  uint8_t addrHigh = (uint8_t)(p2->addr >> 8);
  uint8_t num1Low = (uint8_t)(p2->num1 & 0xFF);
  uint8_t num1High = (uint8_t)(p2->num1 >> 8);

  // ? uint ??? p2->data ??? 4 ? uint8_t ???? 
  uint8_t dataLL = (uint8_t)(p2->data & 0xFF);          // ????
  uint8_t dataLH = (uint8_t)((p2->data >> 8) & 0xFF);   // ????
  uint8_t dataHL = (uint8_t)((p2->data >> 16) & 0xFF);  // ????
  uint8_t dataHH = (uint8_t)((p2->data >> 24) & 0xFF);  // ????

  // ???????????? CRC ?????
  uint8_t dataToCalculateCRC[]= {
    p2->Header, p2->Cmd, addrHigh, addrLow, num1High, num1Low, p2->len, dataLH, dataLL, dataHH,  dataHL
   };
  // ?? CRC16 ???
	size_t dataLen = sizeof(dataToCalculateCRC) / sizeof(dataToCalculateCRC[0]);
  uint16_t crc = CalculateCRC16(dataToCalculateCRC,dataLen );
  p2->CRC_LOW = (uint8_t)(crc & 0xFF);
  p2->CRC_HIGH = (uint8_t)(crc >> 8);

//  uint8_t frameuint8_ts[]  ={           //????,????,????
//  p2->Header, p2->Cmd, addrHigh, addrLow, num1High, num1Low, p2->len, dataLH, dataLL, dataHH,  dataHL, p2->CRC_LOW, p2->CRC_HIGH
//   };
		data[0] = p2->Header;
		data[1] = p2->Cmd;
		data[2] = addrHigh;
		data[3] = addrLow;
		data[4] = num1High;
		data[5]= num1Low;
		data[6] =  p2->len;
		data[7] = dataLH;
		data[8] = dataLL;
		data[9] = dataHH;
		data[10] = dataHL;
		data[11]=  p2->CRC_LOW;
		data[12] = p2->CRC_HIGH; 
}

uint16_t CalculateCRC16(uint8_t *data, size_t len)
{
    uint16_t crc = 0xFFFF;
    
    for (size_t i = 0; i < len; i++)
    {
        crc ^= (uint16_t)data[i];
        
        for (int j = 0; j < 8; j++)
        {
            if (crc & 0x0001)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    
    return crc;
}
//#include "stdafx.h"
#include "crc.h"
  

bool crc16_head_check(uint8_t* buf, uint16_t len)
{
	uint16_t result = 0;
	uint16_t tableNo = 0;

	for (int i = 0; i < len - 2; i++)
	{
		tableNo = ((result & 0xff) ^ (buf[i] & 0xff));
		result = ((result >> 8) & 0xff) ^ CRC16Table[tableNo];
	}
	if ((uint8_t)result == buf[len - 2] && (uint8_t)(result >> 8) == buf[len - 1])
		return 1;
	else
		return 0;
}

uint16_t crc16_make(uint8_t* buf, uint16_t len)
{
	uint16_t result = 0;
	uint16_t tableNo = 0;

	for (int i = 0; i < len - 2; i++)
	{
		tableNo = ((result & 0xff) ^ (buf[i] & 0xff));
		result = ((result >> 8) & 0xff) ^ CRC16Table[tableNo];
	}
	buf[len - 2] = (uint8_t)result;
	buf[len - 1] = (uint8_t)(result >> 8);
	return result;
}

// 生成包头校验码（55 AA + 长度）
void generate_head_crc(uint8_t* buf)
{
	uint16_t result = 0;
	uint16_t tableNo = 0;

	// 计算前4字节的CRC
	for (int i = 0; i < 4; i++) 
	{
		tableNo = ((result & 0xff) ^ (buf[i] & 0xff));
		result = ((result >> 8) & 0xff) ^ CRC16Table[tableNo];
	}

	// 将CRC写入第5-6字节
	buf[4] = (uint8_t)result;        // CRC低8位
	buf[5] = (uint8_t)(result >> 8); // CRC高8位
}
#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"

//#pragma pack (1)  //结构体或联合体，按 1 字节对齐

#define firmware_version 0x01010002

#define Host_computer_ID 0x01
#define Control_board_ID 0x02
#define High_Voltage_board_ID 0x03


#define cmd_set_comm 01
#define cmd_set_app 03
#define cmd_set_PC 02
#define cmd_set_DMS 04



enum Dvice_name {
	PC = 1,
	App = 2,
	Module =3
};

typedef struct
{
	uint8_t sof1;  ////正常值为0x55
	uint8_t sof2;  ////正常值为0xaa
	uint16_t length; /////整个数据包的长度
	uint16_t crc_HeadCheck; ////crc包头校验
}protocol_head;

typedef struct
{
	protocol_head packageHead;
	uint8_t src_ID;
	uint8_t dst_ID;
	uint8_t CMD_set;
	uint8_t CMD_ID;
	uint8_t ack;
	uint8_t res;
}CMD_Pack;





void CMD_handlder_fun(uint8_t* buf);
void CMD_handlder_tx(uint8_t* buf,uint8_t cmd_set,uint8_t cmd_id,uint8_t ack,uint8_t res,	uint8_t len);

#pragma pack ()
#pragma once
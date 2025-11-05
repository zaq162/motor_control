#include "stdint.h"
#include "stdlib.h"
#include "ringbuffer.h"
#include "cmdhandle.h"

//#define UART_LINK

typedef struct
{
	ringbuffer* rbuf;
	uint8_t*  buf;
	uint8_t step;
	uint16_t len;
	uint16_t index;
}decodeData;

typedef struct
{
	ringbuffer* rbuf;
	uint8_t*  image_buf;
	uint8_t step;
	uint16_t pack_num;
	uint16_t pack_count;
	uint16_t index;
	uint16_t max_pack_count;
	uint16_t pack_len;
}image_decodeData;


void unpack_init(decodeData* buf, ringbuffer* rbuf, uint8_t* CMD_buf);
void unpack_fun(decodeData* buf);
#pragma once  //告诉编译器“这个头文件在整个编译过程中最多只被包含一次”，防止重复包含导致的重定义错误。

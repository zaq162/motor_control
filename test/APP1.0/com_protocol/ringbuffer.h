#include "stdlib.h"
#include "stdio.h"
#include "stdint.h"
#include "string.h"
#define RB_bufsz 512
#include <stdbool.h>   
typedef struct
{
	uint8_t *ring_buffer; // 指向实际存放数据的环形缓冲区()shuzu首地址,
	uint16_t rd_index, wr_index;// 当前读指针（下一次读取的位置）// 当前写指针（下一次写入的位置）
	uint16_t len;// 缓冲区总容量（最多可存多少字节）
	uint16_t last_count;  // 最近一次读/写操作实际处理的字节数
}ringbuffer;

void ring_buf_init(ringbuffer* rbuf, uint8_t* buf);
void ring_buf_clear(ringbuffer* rbuf);
bool ring_buf_empty(ringbuffer* rbuf);
bool ring_buf_full(ringbuffer* rbuf);
uint8_t ring_buf_read(ringbuffer* rbuf);
void ring_buf_write(ringbuffer* rbuf, uint8_t data); 
uint16_t ring_buf_avail(ringbuffer* rbuf); //获取环形缓冲区中可读取数据数量
#pragma once

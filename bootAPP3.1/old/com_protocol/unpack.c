//#include "stdint.h"
#include "ringbuffer.h"
#include "unpack.h"
#include "crc.h"
#include "cmdhandle.h"
//#include <afxpriv.h>
#pragma warning(disable:4996)

typedef unsigned char BYTE;
uint16_t pack_len = 0;

void unpack_init(decodeData* buf, ringbuffer* rbuf, uint8_t* CMD_buf)
{
	buf->rbuf = rbuf;
	buf->buf = CMD_buf;
	buf->step = 0;
	buf->index = 0;
}


void unpack_fun(decodeData* buf)
{
	uint8_t data;
	data = ring_buf_read(buf->rbuf);
	switch(buf->step) //包头验证，一次只有一个字节
	{
	case 0:
		if (data == 0x55)
		{
			buf->step = 1;
			buf->buf[buf->index++] = data;
		}
		else
		{
			buf->index = 0;
			buf->step = 0;
		}
			
		break;
	case 1:
		if (data == 0xaa) {
			buf->step = 2;
			buf->buf[buf->index++] = data;
		}
		else
		{
			buf->step = 0;
			buf->index = 0;
		}
		break;
	case 2:
		buf->len = data;
		buf->buf[buf->index++] = data;
		buf->step = 3;
		break;
	case 3:
		buf->buf[buf->index++] = data;
		buf->len = buf->len + ((uint16_t)data << 8);
		buf->step = 4;
		break;
	case 4:
		buf->buf[buf->index++] = data;
		buf->step = 5;
		break;
	case 5:
		buf->buf[buf->index++] = data;
		if (crc16_head_check(buf->buf, sizeof(protocol_head)))
		{
			buf->step = 6;
		}
		else
		{
			buf->step = 0;
			buf->index = 0;
		}
		break;
	case 6:
		buf->buf[buf->index++] = data;
		if (buf->len == buf->index)
		{
			if (crc16_head_check(buf->buf, buf->len))
			{
				CMD_handlder_fun(buf->buf);
			}
			buf->step = 0;
			buf->index = 0;
		}
		else
		{
			buf->step = 6;
		}
		break;
	default:
		buf->step = 0;
		break;
	}

}
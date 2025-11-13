#include "ringbuffer.h"

void ring_buf_init(ringbuffer* rbuf, uint8_t* buf)
{
	rbuf->ring_buffer = buf;
	rbuf->rd_index = 0;
	rbuf->wr_index = 0;
	rbuf->last_count = RB_bufsz;
}

void ring_buf_clear(ringbuffer* rbuf)
{
	rbuf->rd_index = 0;
	rbuf->wr_index = 0;
}


bool ring_buf_empty(ringbuffer* rbuf)
{
	if (ring_buf_full(rbuf))
	{
		rbuf->rd_index = 0;
	}
	return rbuf->rd_index == rbuf->wr_index;
}


bool ring_buf_full(ringbuffer* rbuf)
{
	return rbuf->rd_index == RB_bufsz;
}


uint8_t ring_buf_read(ringbuffer* rbuf)
{
	if (ring_buf_full(rbuf))
	{
		rbuf->rd_index = 0;
	}
	return (rbuf->ring_buffer[rbuf->rd_index++]); //读出当前读指针位置的一个字节，读指针+1
}

void ring_buf_write(ringbuffer* rbuf, uint8_t data)
{
	if (ring_buf_full(rbuf))
	{
		rbuf->wr_index = 0;
	}
	if (rbuf == NULL || rbuf->ring_buffer == NULL|| rbuf->len == 0 || ring_buf_full(rbuf))
	{
		return;
	}

	rbuf->ring_buffer[rbuf->wr_index++] = data;
	//rbuf->wr_index = (rbuf->wr_index + 1) % rbuf->len;
}

uint16_t ring_buf_avail(ringbuffer* rbuf)
{
	if (rbuf == NULL) return 0;
	return (rbuf->rd_index - rbuf->wr_index - 1 + rbuf->len) % rbuf->len;
}


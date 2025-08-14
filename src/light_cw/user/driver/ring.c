#include "ring.h"
#include <stdlib.h>
#include <string.h>
/* 用循环数组实现的队列 */ 

static RING_T ring0_fifo_hd = {0,0,{0}};
static RING_T ring1_fifo_hd = {0,0,{0}};

//初始化队列
void InitRingFIFO(RING_T *queue)
{
	queue->out = 0;
	queue->in  = 0;
	memset(queue->BasicArr, 0, sizeof(uint8_t)*BUFFER_ZISE);
}
//队列是否为空
int IsEmptyRingFIFO(RING_T *queue)
{
	if (queue->out == queue->in)//队首等于队尾
		return 1;
	else
		return 0;
}
//队列是否为满
int IsFullRingFIFO(RING_T *queue)
{
	//队尾下个数据为队首
	//队列真正使用长度为队列长度减一
	if (((queue->in + 1) % BUFFER_ZISE) == queue->out)
		return 1;
	else
		return 0;
}
//入队
int EnterRingFIFO(RING_T *queue, uint8_t *msg)
{
	if (IsFullRingFIFO(queue))
	{
		return 0;
	}

	//从队尾入队
	queue->BasicArr[queue->in] = *msg;//入队数据
	queue->in = (queue->in + 1) % BUFFER_ZISE;//队尾移向下个位置

	return 1;
}
//出队
int OutRingFIFO(RING_T *queue, uint8_t *msg)
{
	if (IsEmptyRingFIFO(queue))
	{
		return 0;
	}

	*msg = queue->BasicArr[queue->out];//出队值
	queue->out = (queue->out + 1) % BUFFER_ZISE;//指向下一个出队值

	return  1;
}


void ring_fifo_init(void)
{
	InitRingFIFO(&ring0_fifo_hd);
	InitRingFIFO(&ring1_fifo_hd);
}

uint8_t ring_fifo_read(ring_index_e index, uint8_t *buf, uint8_t len)
{
	RING_T *p_hd = 0;
	uint8_t read_len = 0;

	if( (index >= ring_max) || (!buf) || (len==0) ) return 0;
	
	if( index == ring_0)
	{
		p_hd = &ring0_fifo_hd;
	}
	else if(index == ring_1)
	{
		p_hd = &ring1_fifo_hd;
	}

	for(read_len=0; read_len<len;)
	{
		if( OutRingFIFO(p_hd, buf+read_len) )
		{
			read_len++;
		}
		else
		{
			return read_len;
		}
	}
	return read_len;
}

uint8_t ring_fifo_write(ring_index_e index, uint8_t *buf, uint8_t len)
{
	RING_T *p_hd = 0;
	uint8_t write_len = 0;

	if( (index >= ring_max) || (!buf) || (len==0) ) return 0;
	
	if( index == ring_0)
	{
		p_hd = &ring0_fifo_hd;
	}
	else if(index == ring_1)
	{
		p_hd = &ring1_fifo_hd;
	}

	for(write_len=0; write_len<len;)
	{	
		if( EnterRingFIFO(p_hd, buf+write_len) )
		{
			write_len++;
		}
		else
		{
			return write_len;
		}
	}
	return write_len;
}

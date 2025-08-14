#ifndef _RING_H_
#define _RING_H_
#include <stdint.h>

//typedef unsigned char uint8_t;
//typedef unsigned short uint16_t;

typedef enum{
	ring_0,
	ring_1,
	ring_max,
}ring_index_e;

#define BUFFER_ZISE	64			//队列长度

typedef struct{
	uint8_t in;					//入队列下标
	uint8_t out;				//出队列下标
	uint8_t BasicArr[BUFFER_ZISE];	//队列数据
}RING_T;

void InitRingFIFO(RING_T *queue);

//bool IsEmptyRingFIFO(Queue *queue);

//bool IsFullRingFIFO(Queue *queue);

int EnterRingFIFO(RING_T *queue, uint8_t *msg);

int OutRingFIFO(RING_T *queue, uint8_t *msg);

void ring_fifo_init(void);

uint8_t ring_fifo_read(ring_index_e index, uint8_t *buf, uint8_t len);

uint8_t ring_fifo_write(ring_index_e index, uint8_t *buf, uint8_t len);

#endif

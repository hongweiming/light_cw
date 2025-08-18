#ifndef _MSG_QUE_H_
#define _MSG_QUE_H_
#include <stdint.h>


//typedef unsigned char uint8_t;
//typedef unsigned short uint16_t;

#define APP_FRAME_MAX_LEN    (64)
#define QUEUE_ZISE	10			//队列长度


typedef struct{
	uint8_t in;					//入队列下标
	uint8_t out;				//出队列下标
	uint32_t BasicArr[QUEUE_ZISE];	//队列数据
}Queue;

void InitQueue(Queue *queue);

//bool IsEmptyQueue(Queue *queue);

//bool IsFullQueue(Queue *queue);

int EnterQueue(Queue *queue, void *msg);

int OutQueue(Queue *queue, void *msg);

#endif

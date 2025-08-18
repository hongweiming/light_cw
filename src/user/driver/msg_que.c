#include "msg_que.h"
#include <stdlib.h>
#include <string.h>

/* 用循环数组实现的队列 */ 
/* 此方式只适用于32位单片机 不适用于8位单片机 */
//初始化队列
void InitQueue(Queue *queue)
{
	queue->out = 0;
	queue->in  = 0;
	memset(queue->BasicArr, 0, sizeof(uint32_t)*QUEUE_ZISE);
}
//队列是否为空
int IsEmptyQueue(Queue *queue)
{
	if (queue->out == queue->in)//队首等于队尾
		return 1;
	else
		return 0;
}
//队列是否为满
int IsFullQueue(Queue *queue)
{
	//队尾下个数据为队首
	//队列真正使用长度为队列长度减一
	if (((queue->in + 1) % QUEUE_ZISE) == queue->out)
		return 1;
	else
		return 0;
}
//入队
int EnterQueue(Queue *queue, void *msg)
{
	if (IsFullQueue(queue))
	{
		return 0;
	}
	
	//从队尾入队
	queue->BasicArr[queue->in] = *((uint32_t *)msg);//入队数据
	queue->in = (queue->in + 1) % QUEUE_ZISE;//队尾移向下个位置

	return 1;
}
//出队
int OutQueue(Queue *queue, void *msg)
{
	if (IsEmptyQueue(queue))
	{
		return 0;
	}

	*((uint32_t *)msg) = queue->BasicArr[queue->out];//出队值
	queue->out = (queue->out + 1) % QUEUE_ZISE;//指向下一个出队值

	return  1;
}

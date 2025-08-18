#include "com_manage.h"
#include <stdlib.h>
#include <string.h>
#include "msg_que.h"
#include "ring.h"
#include <stdio.h>
#include "usart.h"
#include "led.h"
#include "le_be.h"

typedef struct{
	uint8_t *dat;
	uint16_t len;
	void (*callback)(uint8_t *buf, uint16_t len);
}msg_t;

typedef struct{
	unsigned char buf[APP_FRAME_MAX_LEN];
	unsigned char len;
	unsigned char timeout;
}uart_recv_t;

static uart_recv_t uart_rx_hd = {{0},0,0};
//缓存
static unsigned char rx_data[APP_FRAME_MAX_LEN] = {0};//接收buff
//static unsigned char tx_data[APP_FRAME_MAX_LEN] = {0};//发送buff

static unsigned char com_init = 0;
static unsigned char sg_seq = 0;

Queue tx_mq_hd;
Queue rx_mq_hd;

msg_t *tx_msg = NULL;
msg_t *rx_msg = NULL;

#define LOG_HEX(raw, len)    do{ const int l=(len); int x;                          \
                                 for(x=0 ; x<l ; x++) printf("0x%02x ",*((raw)+x)); \
                                 printf("\r\n");}while(0)

//void com_uart_recv_process(uint8_t *buf, uint16_t len);
//void com_message_post(Queue *que_hd, void(*cb)(uint8_t *buf, uint16_t len), uint8_t *buf, uint16_t len);

/*
 * @brief:   crc8 calculate
 *
 * */
uint8_t crc8Calculate(uint16_t type, uint16_t length, uint8_t *data)
{
	uint16_t n;
	uint8_t crc8;
	crc8  = (type   >> 0) & 0xff;
	crc8 ^= (type   >> 8) & 0xff;
	crc8 ^= (length >> 0) & 0xff;
	crc8 ^= (length >> 8) & 0xff;
	for(n = 0; n < length; n++)	
	{
		crc8 ^= data[n];
	}
	return crc8;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
void app_cmd_ff01(uint8_t *buf, uint16_t len)
{
	serial_packet_t *msg = (serial_packet_t *)buf;

	printf("%s %d .\r\n", __func__,__LINE__);
	uint16_t msgtype = get_be_word((uint8_t *)&msg->msgtype);
	uint16_t msglen = get_be_word((uint8_t *)&msg->msglen);
	printf("msgtype %04x.\r\n", msgtype);
	LOG_HEX(msg->dat,msglen);

	if( msg->msglen >= 10 )
	{
		app_packet_t *pkt = (app_packet_t *)(msg->dat);
		if((pkt->frame_head==APP_FRAME_HEAD)&&(pkt->board_type==APP_BOARD_TYPE))
		{
			uint16_t cw;
			uint8_t brightness;
			printf("pkt->op_code %02x.\r\n", pkt->op_code);
			switch(pkt->op_code)
			{
				case APP_OP_CODE_OFF:
					app_led_turn_off();
				break;

				case APP_OP_CODE_ON:
					cw = get_be_word(pkt->data+1);
					brightness = pkt->data[3];
					printf("cw %d brightness %d.\r\n",cw,brightness);
					app_led_turn_on(cw,brightness);
				break;

				case APP_OP_CODE_SET_COLOR:
					cw = get_be_word(pkt->data+1);
					brightness = pkt->data[3];
					printf("cw %d brightness %d.\r\n", cw,brightness);
					app_led_cw_brightness_set(cw, brightness);
				break;

				case APP_OP_CODE_GET_COLOR:
					app_led_cw_brightness_get();
				break;

				default:
					printf("unkonw op_code \r\n");
				break;
			}
		}
	}
}

void app_cmd_8000(uint8_t *buf, uint16_t len)
{
	serial_packet_t *msg = (serial_packet_t *)buf;

	printf("%s %d .\r\n", __func__,__LINE__);
	uint16_t msgtype = get_be_word((uint8_t *)&msg->msgtype);
	uint16_t msglen = get_be_word((uint8_t *)&msg->msglen);
	printf("msgtype %04x.\r\n", msgtype);
	LOG_HEX(msg->dat,msglen);
}

typedef struct{
	uint16_t cmd;
	void (*docmd)(uint8_t *buf, uint16_t len);
}cmd_table_t;

cmd_table_t cmd_table[2] = {
							{0xff01,app_cmd_ff01}, //无线数据接收
							{0x8000,app_cmd_8000}, //应答帧
							};

void com_rx_message_analy(uint8_t *buf, uint16_t len)
{
	serial_packet_t *pmsg = (serial_packet_t *)buf;
	uint8_t table_size = sizeof(cmd_table)/sizeof(cmd_table[0]);
	uint8_t i=0;
	cmd_table_t *cmd=NULL;
	uint16_t cmdtype;
	
	if( (buf == NULL) || (len == 0) ) return;

	for(i=0; i<table_size; i++ )
	{
		cmdtype = get_be_word((uint8_t *)(&pmsg->msgtype));
		if(cmdtype == cmd_table[i].cmd)
		{
			cmd = &cmd_table[i];
			break;
		}  
	}

	if(i==table_size)
	{	printf("[%s %d] %04x unknow cmdtype\r\n",__func__,__LINE__,cmdtype);
		return;
	}

	if(cmd)
	{
		if(cmd->docmd)
		{
			cmd->docmd(buf, len);
		}
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////
void com_message_post(Queue *que_hd, void(*cb)(uint8_t *buf, uint16_t len), uint8_t *buf, uint16_t len)
{
	msg_t *msg;
	msg = malloc(sizeof(msg_t) + len);
	if(msg == NULL)
	{
		return;
	}

	msg->dat = (uint8_t *)(msg + 1);
	memcpy(msg->dat, buf, len);
	msg->len = len;
	msg->callback = cb;
	if(EnterQueue(que_hd, &msg)==0)
	{
		free(msg);
	}
}
//组成一帧完整数据帧
void com_message_frame_packet(uint16_t msgtype, uint16_t msglen, uint8_t *dat)
{
	uint16_t plen = PRORO_ZG_UART_FRAME_MIN + msglen;
	uint8_t *pbuf = malloc(plen);
	serial_packet_t *pkt = (serial_packet_t *)pbuf;

	if(!pbuf) return;

	//组帧
	pkt->startflag = PROTO_ZG_UART_FRAME_HEAD;
	set_be_word((uint8_t *)(&pkt->msgtype), msgtype);
	set_be_word((uint8_t *)(&pkt->msglen), msglen);
    pkt->crc8 = crc8Calculate(msgtype, msglen, dat);
    for(uint16_t i=0; i<msglen; i++)
    {
        pkt->dat[i] = dat[i];
    }
    pbuf[plen-1] = PROTO_ZG_UART_FRAME_END;
	//入发送队列
	com_message_post(&tx_mq_hd, NULL, pbuf, plen);
	free(pbuf);
}
//增加地址信息
void com_message_send_with_shortaddr(unsigned char *buf, unsigned char len, unsigned char seq, cmd_type_e cmd_type)
{
	uint8_t plen = sizeof(addr_info_short_t) + len;
	uint8_t *pbuf = malloc(plen);
	addr_info_short_t *pkt = (addr_info_short_t *)pbuf;

	if(!pbuf) return;

	//填充地址信息
	pkt->addr_mode = ADDR_MODE_SHORT;
	pkt->dst_addr = 0x0000;
	pkt->src_ep = ENDPOINT_SRC;
	pkt->dst_ep = ENDPOINT_DST;
	pkt->len = len;
	uint8_t *dat = pbuf + sizeof(addr_info_short_t);
	//数据拷贝
	for(uint8_t i=0; i<len; i++)
	{
		dat[i] = buf[i];
	}
	com_message_frame_packet(CMDID_PASS_THROUGH_SEND, plen, pbuf);
	free(pbuf);
}

//灯带状态回传
void com_cw_report(unsigned short cw, unsigned short brightness)
{
	uint8_t buf[8];
	app_packet_t *pkt = (app_packet_t *)buf;
	pkt->frame_head = APP_FRAME_HEAD;
	pkt->board_type = APP_BOARD_TYPE;
	pkt->device_id = APP_DEVICE_ID;
	pkt->op_code = APP_OP_CODE_RPT;
	pkt->data[0] = led_get_on_off_status();
	pkt->data[1] = cw>>8;
	pkt->data[2] = cw;
	pkt->data[3] = brightness;
	com_message_send_with_shortaddr(buf, sizeof(buf), 0X00, REQ_CMD);
	printf("com_cw_report:onoff %d, cw %d, brightness %d.\r\n", led_get_on_off_status(),cw,brightness);
}

void com_uart_recv_process(uint8_t *buf, uint16_t len)
{
	// static unsigned char seq = 0xff;
	// static unsigned char rx_first = 1;
	// serial_packet_t *pbuf = (serial_packet_t *)buf;

	// if((rx_first==0)&&(seq==pbuf->seq))
	// {
	// 	return;
	// }

	// if(rx_first)
	// {
	// 	rx_first = 0;
	// }
	// seq = pbuf->seq;
	com_message_post(&rx_mq_hd, com_rx_message_analy, buf, len);
	printf("uart recv %d bytes <------:", len);
	LOG_HEX(buf, len);
}

void com_mq_init(void)
{
	InitQueue(&tx_mq_hd);
	InitQueue(&rx_mq_hd);
	ring_fifo_init();
}
//串口发送任务
void com_tx_task_10ms(void)
{
	if(!com_init)
	{
		com_mq_init();
		tx_msg = NULL;
		rx_msg = NULL;
		com_init = 1;
		sg_seq = 0;
		return;
	}

	if(OutQueue(&tx_mq_hd, &tx_msg))
	{
		if(tx_msg->len)
		{
			HAL_UART_Transmit(&huart2, tx_msg->dat, tx_msg->len, 1000);
		}
		printf("uart send %d bytes ------>:", tx_msg->len);
		LOG_HEX(tx_msg->dat, tx_msg->len);
		free(tx_msg);
		tx_msg = NULL;
	}
}
//串口接收任务
void com_rx_task_10ms(void)
{
	unsigned char len = 0;
	unsigned char i = 0;

	if(!com_init)
	{
		return;
	}

	len = ring_fifo_read(ring_0, rx_data, APP_FRAME_MAX_LEN);
	if( len )
	{
		for(i=0; i<len; i++)
		{
			if( uart_rx_hd.len==0 )
			{
				if( rx_data[i]==PROTO_ZG_UART_FRAME_HEAD )
				{
					uart_rx_hd.buf[uart_rx_hd.len++] = rx_data[i];
				}
			}
			else
			{
				uart_rx_hd.buf[uart_rx_hd.len++] = rx_data[i];
				if( uart_rx_hd.len >= PRORO_ZG_UART_FRAME_MIN )
				{
					serial_packet_t *hdr;
					uint16_t msglen;

					hdr = (serial_packet_t *)(uart_rx_hd.buf);
					msglen = get_be_word((uint8_t *)(&hdr->msglen));
					if( uart_rx_hd.len == msglen + PRORO_ZG_UART_FRAME_MIN )	//receive len ok
					{
						uint16_t msgtype = get_be_word((uint8_t *)&hdr->msgtype);
						uint8_t crc = crc8Calculate(msgtype, msglen, hdr->dat);
						if( crc == hdr->crc8 )
						{
							com_uart_recv_process(uart_rx_hd.buf, uart_rx_hd.len); //receive on frame and crc check ok
						}
						uart_rx_hd.len = 0;
						uart_rx_hd.timeout = 0;
					}
				}
			}
		}
	}
	else
	{
		if(uart_rx_hd.len > 0)
		{
			if(uart_rx_hd.timeout++ > 200)
			{
				uart_rx_hd.len = 0;
				uart_rx_hd.timeout = 0;
			}
		}
	}
}
//接收任务 执行回调函数
void com_oam_task(void)
{
	if(!com_init)
	{
		return;
	}

	if( OutQueue(&rx_mq_hd, &rx_msg) )
	{
		if(rx_msg->callback)
		{
			rx_msg->callback(rx_msg->dat, rx_msg->len);
		}
		free(rx_msg);
		rx_msg = NULL;
	}
}

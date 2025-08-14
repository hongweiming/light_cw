#ifndef _COM_MANAGE_H_
#define _COM_MANAGE_H_

#include <stdint.h>

//cmdid
#define CMDID_PASS_THROUGH_SEND     0X0044
#define CMDID_PASS_THROUGH_RECEIVE  0XFF01
#define CMDID_RSP                   0X8000

//address mode
#define ADDR_MODE_SHORT 0X02
#define ADDR_MODE_IEEE  0X03
#define ADDR_MODE_LONG  0X03

//endpoint
#define ENDPOINT_SRC    0X01
#define ENDPOINT_DST    0X01

//app cmdid
typedef enum 
{
	CMDID_SET_CW 	= 0x50,
	CMDID_RPT_CW 	= 0X51,
}cmdid_e;

typedef enum{
    REQ_CMD = 0,
    RSP_CMD = 1,
}cmd_type_e;

#define PRORO_ZG_UART_FRAME_MIN     7
#define PROTO_ZG_UART_FRAME_MAX     64
#define PROTO_ZG_UART_FRAME_HEAD    0X55 
#define PROTO_ZG_UART_FRAME_END     0XAA

#define APP_FRAME_HEAD              0XFA
#define APP_BOARD_TYPE              0X03
#define APP_DEVICE_ID               0X01

#define APP_OP_CODE_OFF             0X00
#define APP_OP_CODE_ON              0X01  
#define APP_OP_CODE_SET_COLOR       0X02  
#define APP_OP_CODE_GET_COLOR       0X03
#define APP_OP_CODE_RPT             0X00



#pragma pack(1)
typedef struct{
	uint8_t     startflag;
	uint16_t    msgtype;
	uint16_t	msglen;
	uint8_t	    crc8;
	uint8_t	    dat[1];
}serial_packet_t;

typedef struct{
    uint8_t addr_mode;
    uint16_t dst_addr;
    uint8_t src_ep;
    uint8_t dst_ep;
    uint8_t len;
}addr_info_short_t;

typedef struct{
    uint8_t addr_mode;
    uint8_t dst_addr[8];
    uint8_t src_ep;
    uint8_t dst_ep;
    uint8_t len;
}addr_info_ieee_t;

typedef struct{
  uint8_t   frame_head;
  uint8_t   board_type;
  uint8_t   device_id;
  uint8_t   op_code;
  uint8_t   data[1];
}app_packet_t;

#pragma pack()


void com_message_frame_packet(uint16_t msgtype, uint16_t msglen, uint8_t *dat);

void com_cw_report(unsigned short cw, unsigned short brightness);

void com_tx_task_10ms(void);

void com_rx_task_10ms(void);

void com_oam_task(void);

#endif

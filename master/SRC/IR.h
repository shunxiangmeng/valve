#ifndef _IR_H
#define _IR_H

#include "misc.h"

#define BIND_VALVE_ID	"FFFFEE"

#define UART1_RX_MAX_LEN	0x20

typedef struct 
{
	uint8_t rxCount;
	uint8_t rxFlag;
	uint8_t rxCmd;
	uint8_t lastCmd;
	uint32_t randomCode;
	uint8_t rxBuf[UART1_RX_MAX_LEN];
	uint8_t rxData[UART1_RX_MAX_LEN];
	char valveID[8];
}IR_INFO;





typedef struct
{
	uint8_t head;  //'@'
	uint8_t cmd;
	uint8_t len;   //数据长度
	uint8_t ramdomCode[2];
	uint8_t data[10];
	uint8_t checkSum;
	uint8_t tail;  //0x23	
}IR_SENDCMD;



extern IR_INFO gIR;
void IR_Init(void);
void IR_SendCMD(uint8_t cmd, uint32_t randomCode, uint8_t *dat, uint8_t len);
void IR_SendAck(uint32_t randomCode, uint8_t ok);
int IR_GetValveID(uint32_t timeOut);
void IR_clear(void);

#endif

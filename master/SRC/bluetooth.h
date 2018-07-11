#ifndef _BLUETOOTH_H
#define _BLUETOOTH_H

#include "stm32f10x.h"
#include "usart3.h"

#define BLE_COM_RX_CNT_MAX		128
#define BLE_COM_TX_CNT_MAX      64


typedef struct
{
	u32 rxTime;  //response time(ms)
	u32 rxCount;
	u8  txBuf[BLE_COM_TX_CNT_MAX];
	u8  rxBuf[BLE_COM_RX_CNT_MAX];
}BLE_COM;



int BLE_Init(void);
int BLE_sendAT(char *sendStr, char *searchStr, u32 outTime);

#endif


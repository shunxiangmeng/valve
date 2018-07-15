#ifndef _BLUETOOTH_H
#define _BLUETOOTH_H

#include "stm32f10x.h"
#include "usart3.h"

#define BLE_COM_RX_CNT_MAX		128
#define BLE_COM_TX_CNT_MAX      64


typedef struct
{
	u8  isUartOk;   //ģ���Ƿ�ok
	u8  connectFlag;    //�������ӱ�־λ
	char  mac[16];
	char  name[32];
	char  version[16];
	
	u32 rxTime;  //response time(ms)
	u32 rxCount;
	u8  rxFlag;
	u8  rxCmd;
	u8  txBuf[BLE_COM_TX_CNT_MAX];
	u8  rxBuf[BLE_COM_RX_CNT_MAX];
}BLE_INFO;


extern BLE_INFO gBLE;

unsigned char CheckSum(const u8 *p, const u8 n);

void BLE_RxStatus(FunctionalState status);
int BLE_Init(void);
int BLE_BindInit(void);
int BLE_sendAT(char *sendStr, char *searchStr, u32 outTime);
void BLE_GetInfo(const char *str, char *out, char outLen);
void BLE_SendData(uint8_t cmd, char *data, uint8_t len);
int BLE_GetConnectStatus(void);
void BLE_Clear(void);

#endif


#ifndef _BLUETOOTH_H
#define _BLUETOOTH_H

#include "stm32f10x.h"
#include "usart3.h"

#define BLE_COM_RX_CNT_MAX		128
#define BLE_COM_TX_CNT_MAX      64


typedef struct
{
	u8  isUartOk;       //蓝牙串口连接是否ok
	u8  connectFlag;    //蓝牙连接状态
	char  mac[16];      //蓝牙mac
	char  conenctMac[16];//连接上的mac
	char  name[32];
	char  version[16];
	
	char  AdMac1[16];   //绑定mac1
	char  AdMac2[16];   //绑定mac2
	char  AdMac3[16];   //绑定mac3
	
	u32 rxTime;         //response time(ms)
	u32 rxCount;
	u8  rxFlag;
	u8  rxCmd;
	char  txBuf[BLE_COM_TX_CNT_MAX];
	char  rxBuf[BLE_COM_RX_CNT_MAX];
	char  rxPassword[8];
	char  date[8];
}BLE_INFO;


typedef struct
{
	uint32_t isBinded;        //蓝牙绑定标志位
	uint32_t count;           //绑定的个数
	char     bindMac[3][16];  //绑定的小程序地址1
	char     stationPassword[8];     //六位密码823492
	uint8_t  chechSum;        //校验码
	char     res[3];
}BLE_BIND_INFO;


unsigned char CheckSum(const u8 *p, const u8 n);

void BLE_RxStatus(FunctionalState status);
int BLE_Init(void);
int BLE_BindInit(void);
int BLE_sendAT(char *sendStr, char *searchStr, u32 outTime);
void BLE_GetInfo(const char *str, char *out, char outLen);
void BLE_SendData(uint8_t cmd, char *data, uint8_t len);
int BLE_GetConnectStatus(void);
void BLE_Clear(void);

void BLE_WriteBindInfo(void);
void BLE_ReadBindInfo(void);
int  BLE_Bind(char* mac);
int BLE_BindClean(void);

#endif


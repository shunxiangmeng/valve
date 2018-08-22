#ifndef _BLUETOOTH_H
#define _BLUETOOTH_H

#include "stm32f10x.h"
#include "usart3.h"
#include "config.h"

#define BLE_COM_RX_CNT_MAX		128
#define BLE_COM_TX_CNT_MAX      64


typedef struct
{
	char ok[2];
	char plus[1];
	char res[4];
	char type[1];
}BLE_AT_TYPE;

typedef struct
{
	char ok[2];
	char plus[1];
	char res[4];
	char password[6];
}BLE_AT_PASSWORD;

typedef struct
{
	char res[3];
//	char password[6];    //获取阀门ID时，去掉密码字段
}BLE_CMD_GETVALVEID;

typedef struct
{
	char res[3];
	char password[6];
}BLE_CMD_LOGIN;

typedef struct
{
	char res[3];
	char date[6];
	char passwrod[6];
	char valveId[6];
	char result;     //服务器成功保存标志（1）（0为成功，1为失败）
}BLE_CMD_WRITEDATE;

typedef struct
{
	char res[3];
	char password_old[6];  //原始密码
	char password_new[6];  //新密码
}BLE_CMD_SETBLEPASS;       //设置蓝牙密码

typedef union
{
	char buf[BLE_COM_TX_CNT_MAX];
	BLE_AT_TYPE      	atConnectType;        //连接鉴权模式
	BLE_AT_PASSWORD  	atPassword;
	
	BLE_CMD_GETVALVEID  cmdGetValveId;
	BLE_CMD_WRITEDATE   cmdWriteDate;         //写入有效使用日期
	BLE_CMD_LOGIN       cmdLogin;             //登陆
	
	BLE_CMD_SETBLEPASS  cmdSetBlePassword;

}BLE_RX;


typedef struct
{
	char ret;
	char valveId[6];
}BLE_SEND_VALVEID;


typedef union
{
	char buf[BLE_COM_TX_CNT_MAX];
	BLE_SEND_VALVEID 	sendValveId;
	
}BLE_TX;

typedef struct
{
	char  isUartOk;       //蓝牙串口连接是否ok
	char  isConnect;      //蓝牙连接状态
	char  isLogin;        //登陆标志位
	char  mac[16];        //蓝牙mac
	char  conenctMac[16]; //连接上的mac
	char  name[32];
	char  version[16];
	char  password[8];
	
	u32 rxTime;         //response time(ms)
	u32 rxCount;
	u8  rxFlag;
	u8  rxCmd;
	
	BLE_RX rxData;
	
	char  txBuf[BLE_COM_TX_CNT_MAX];
}BLE_INFO;


//需要保存到flash的密码信息
typedef struct
{
	char     stationPassword[8];     //六位密码823492
	uint8_t  chechSum;               //校验码
	char     res[3];
}STATION_PASSWORD_INFO;


unsigned char CheckSum(u8 *p, const u8 n);

void BLE_RxStatus(FunctionalState status);
int BLE_Init(void);
int BLE_BindInit(void);
int BLE_sendAT(char *sendStr, char *searchStr, u32 outTime);
void BLE_GetInfo(const char *str, char *out, char outLen);
void BLE_SendData(uint8_t cmd, char *data, uint8_t len);
int BLE_GetConnectStatus(void);
void BLE_Clear(void);

void BLE_WriteSationPasswordInfo(void);
void BLE_ReadSationPasswordInfo(void);
int  BLE_Bind(char* mac);
int BLE_BindClean(void);
int BLE_SetStationPassword(char *pass);
int BLE_InitPassword(void);
void print_bleRx(void);

#endif


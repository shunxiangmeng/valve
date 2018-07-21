
#ifndef __BSP_USART1_H
#define __BSP_USART1_H

#include "stm32f0xx.h"
#include <stdio.h>

  
  
/* 宏定义 --------------------------------------------------------------------*/

#define MAXCNTRX1 64
/* 结构体定义 --------------------------------------------------------------------*/
typedef struct 
{
    unsigned char  Sync;        //"@"
    unsigned char  Cmd;        //命令
    unsigned char  Len;       
    unsigned char  RamdomCode[2];
    unsigned char  StationPassword[6];
    unsigned char  Cs; 
    unsigned char  Tail;   
}OPEN_VALVE;


typedef struct 
{
    unsigned char  Sync;        //"@"
    unsigned char  Cmd;        //命令
    unsigned char  Len; 
    unsigned char  RamdomCode[2];
    unsigned char  Data[6];
    unsigned char  Cs; 
    unsigned char  Tail; 
}WRITE_STATION_PASSWORD;


typedef struct 
{
    unsigned char  Sync;        //"@"
    unsigned char  Cmd;        //命令
    unsigned char  Len; 
    unsigned char  RamdomCode[2];
    unsigned char  Password[6];
	unsigned char  ValveID[6];
	unsigned char  FlaskTime[6];
    unsigned char  Cs; 
    unsigned char  Tail; 
}WRITE__FLASK_TIME_AND_PASSWORD;



typedef struct 
{
    unsigned char  Sync;        //"@"
    unsigned char  Cmd;        //命令
    unsigned char  Len; 
    unsigned char  RamdomCode[2];
    unsigned char  Data[4];
    unsigned char  Cs; 
    unsigned char  Tail; 
}WRITE_VALVE_TIME;


typedef struct 
{
    unsigned char  Sync;        //"@"
    unsigned char  Cmd;        //命令
    unsigned char  Len;  
    unsigned char  RamdomCode[2];
    unsigned char  Data[4];
    unsigned char  Cs; 
    unsigned char  Tail; 
}WRITE_FLASK_TIME;

typedef struct 
{
    unsigned char  Sync;        //"@"
    unsigned char  Cmd;        //命令
    unsigned char  Len;  
    unsigned char  RamdomCode[2];
    unsigned char  Data[6];
    unsigned char  Cs; 
    unsigned char  Tail; 
}WRITE_VALVE_ID;

typedef struct 
{
    unsigned char  Sync;        //"@"
    unsigned char  Cmd;        //命令
    unsigned char  Len;  
    unsigned char  RamdomCode[2];
	  unsigned char  ValveId[6];
	  unsigned char  ValveTime[4];
    unsigned char  Cs; 
    unsigned char  Tail; 
}WRITE_VALVE_ID_AND_TIME;


typedef struct 
{
    unsigned char  Sync;        //"@"
    unsigned char  Cmd;        //命令
    unsigned char  Len; 
    unsigned char  RamdomCode[2];
    unsigned char  Data[12];
    unsigned char  Cs; 
    unsigned char  Tail; 
    
}WRITE_SUPER_PASSWORD;

typedef struct 
{
    unsigned char  Sync;        //"@"
    unsigned char  Cmd;        //命令
    unsigned char  Len;       
    unsigned char  RamdomCode[2];
    unsigned char  Result;
    unsigned char  Cs; 
    unsigned char  Tail;   
}COMM_FINISH;



typedef union 	 
{
    unsigned char  Buf[MAXCNTRX1];
    OPEN_VALVE OpenValve;
    COMM_FINISH Finish;
    WRITE_STATION_PASSWORD Password;
    WRITE_VALVE_TIME ValveTime;
    WRITE_FLASK_TIME FlaskTime;
    WRITE_VALVE_ID   ValveId;
    WRITE_SUPER_PASSWORD SuperPassword;  
	WRITE__FLASK_TIME_AND_PASSWORD FlaskTimeAndPassword;
	WRITE_VALVE_ID_AND_TIME ValveIdTime;
    
}IRDA_RECEIVE;        //接收1


typedef struct 
{
    unsigned char  Sync;        //"@"
    unsigned char  Cmd;        //命令
    unsigned char  Len; 
    unsigned char  RamdomCode[2];
    unsigned char  Cs; 
    unsigned char  Tail; 
}RESPONSE0;


typedef struct 
{
    unsigned char  Sync;        //"@"
    unsigned char  Cmd;        //命令
    unsigned char  Len; 
    unsigned char  RamdomCode[2];
    unsigned char  ValveId[6];
    unsigned char  Result;
    unsigned char  Cs; 
    unsigned char  Tail; 
}RESPONSE1;

typedef struct 
{
    unsigned char  Sync;        //"@"
    unsigned char  Cmd;        //命令
    unsigned char  Len; 
    unsigned char  RamdomCode[2];
    unsigned char  Result;
    unsigned char  Cs; 
    unsigned char  Tail; 
}RESPONSE2;


typedef union 
{
    unsigned char  Buf[MAXCNTRX1];
    RESPONSE0 R0;
    RESPONSE1 R1;
    RESPONSE2 R2;
}IRDA_SEND;


typedef struct  {
    uint8_t RxCnt;  //接收数量     
    uint8_t RxFlag;//接收状态标记
    uint8_t ReceiveTime;
    uint8_t TxFlag;//接收状态标记
    IRDA_RECEIVE Rxd;              //终端数据接收	 
    IRDA_SEND Txd; 
    void (*ResetFunc)(void);
    int8_t (*WaitFunc)(uint16_t);
    uint8_t (*SendFunc)(uint8_t * ,uint8_t);
}IRDA_COMM;

/* 函数申明 ------------------------------------------------------------------*/
void UART_Initializes(void);
void UART1_SendByte(uint8_t Data);
uint8_t UART1_SendNByte(uint8_t *pData, uint8_t Length);
void UART1_Printf(uint8_t *String);
void UART1_Clear(void);
int8_t UART1_Wait(uint16_t DelayCnt) ;
void USART1_RxISR(void);

unsigned char GetCheckSum(const uint8_t *p,const uint8_t n); 
  
  

void USART1_Init(void);

#endif

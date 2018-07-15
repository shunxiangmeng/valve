#include "bluetooth.h"
#include "SysTick.h"
#include "string.h"
#include "stdio.h"

#define STATIONNAME		"Warning008"

BLE_INFO gBLE;

void BLE_clear(void);

int BLE_Init(void)
{
	
	Uart3_Init(9600);
	
//RESTART:		
	gBLE.connectFlag = 0;   //蓝牙未连接
	
	if(BLE_sendAT("AT", "OK", 500) != 0)
	{
		Delay_ms(100);
		if(BLE_sendAT("AT", "OK", 500) != 0)
		{
			gBLE.isUartOk = 0;
			return -1;  //初始化失败，模块连接测试失败
		}
	}
	else
	{
		gBLE.isUartOk = 1;  //蓝牙模块连接测试OK
	}
	
	BLE_sendAT("AT+NAME"STATIONNAME, "...", 100);  //设置通知上位机连接状态
	BLE_sendAT("AT+IMME0", "...", 100);             //上电立即工作
	BLE_sendAT("AT+ROLE0", "...", 100);             //设置为主模式，500ms后会重启
	Delay_ms(800);

	//获取mac
	BLE_GetInfo("ADDR", gBLE.mac, sizeof(gBLE.mac));
	//获取name 
	BLE_GetInfo("NAME", gBLE.name, sizeof(gBLE.name));
	//获取版本 
	BLE_GetInfo("VERS", gBLE.version, sizeof(gBLE.version));
	
	#if 0
	if(BLE_sendAT("AT+ROLE?", "OK+Get:", 100) == 0)
	{
		Delay_ms(50);//再接收数据
		if (gBLE.rxBuf[sizeof("OK+Get:") - 1] != '0')
		{
			BLE_sendAT("AT+ROLE0", "...", 200); //设置为主模式，500ms后会重启
			Delay_ms(1000);
			goto RESTART;
		}
	}
	#endif
	
	BLE_sendAT("AT+NOTI1", "...", 200);  //设置通知上位机连接状态
	BLE_sendAT("AT+NOTP1", "...", 200);  //连接成功后模块会发送，”OK+CONN:001122334455”字符
	
	gBLE.connectFlag = 0;   //蓝牙未连接
	BLE_Clear();
	return 0;
}

//==========================================================================================
//蓝牙绑定初始化
int BLE_BindInit(void)
{
	Uart3_Init(9600);
	
	if(BLE_sendAT("AT", "OK", 500) != 0)
	{
		Delay_ms(100);
		if(BLE_sendAT("AT", "OK", 500) != 0)
		{
			gBLE.isUartOk = 0;
			return -1;  //初始化失败，模块连接测试失败
		}
	}
	else
	{
		gBLE.isUartOk = 1;  //蓝牙模块连接测试OK
	}
	
	BLE_sendAT("AT+NAME"STATIONNAME, "...", 100);  //设置蓝牙节点名称
	BLE_sendAT("AT+IMME0", "...", 100);            //工作模式为上电立即工作
	BLE_sendAT("AT+NOTI1", "...", 100);            //设置通知上位机连接状态
	BLE_sendAT("AT+NOTP1", "...", 100);            //连接成功后模块会发送，”OK+CONN:001122334455”字符
	BLE_sendAT("AT+ERASE", "...", 100);            //擦出之前的绑定信息
	BLE_sendAT("AT+TYPE0", "...", 100);            //配对并绑定鉴权模式
//	BLE_sendAT("AT+RENEW", "...", 100);            //恢复出厂设置
	BLE_sendAT("AT+ROLE0", "...", 100);            //设置为主模式，500ms后会重启
	printf("蓝牙重启...\r\n");
	Delay_ms(1000);                                //等待重启
	
	if(BLE_sendAT("AT", "OK", 500) != 0)
	{
		Delay_ms(100);
		if(BLE_sendAT("AT", "OK", 500) != 0)
		{
			printf("蓝牙重启失败\r\n");
			return -1;
		}
	}
	printf("reboot ok\r\n");
	
	//获取name
	BLE_GetInfo("NAME", gBLE.name, sizeof(gBLE.name));
	BLE_Clear();
	
#if 0	
//	while(1)
	{
		Delay_ms(1);
		Delay_ms(1);
	}
#endif
	return 0;
}

void BLE_RxStatus(FunctionalState status)
{
	USART_Cmd(USART3, status);
}

//=========================================
void BLE_Clear(void)
{
    u32 i;
    gBLE.rxCount = 0;			//接收计数器清零
	gBLE.rxTime = 0;
	gBLE.rxFlag = 0;
	gBLE.rxCmd = 0;
    for(i = 0; i < BLE_COM_RX_CNT_MAX; i++)
    {
        gBLE.rxBuf[i] = 0;
    }
}

int BLE_sendAT(char *sendStr, char *searchStr, u32 outTime)
{
    int ret = 0;
    char *pfind = 0;
	
	BLE_Clear();
	Uart3_sendStr(sendStr);
	
    if(searchStr && outTime)//当searchStr和outTime不为0时才等待应答
    {
        while((--outTime)&&(pfind == 0))//等待指定的应答或超时时间到来
        {
			pfind = strstr((char*)gBLE.rxBuf, searchStr);
			if(pfind != 0)
			{
				break;
			}
			Delay_ms(1);
			gBLE.rxTime++;
        }
        if(outTime == 0) 
		{
            ret = -1;    //超时
        }
        if(pfind != 0)//res不为0证明收到指定应答
        {
            ret = 0;
        }
    }
	Delay_ms(5);  //继续接收数据
    return ret;
}



void BLE_SendData(uint8_t cmd, char *data, uint8_t len)
{
	char buf[32] = {0};
	uint32_t cnt = 0;
	uint32_t i = 0;
	uint8_t checkSum = 0;
	
	buf[cnt++] = 0x40;
	buf[cnt++] = cmd;
	buf[cnt++] = len;
	
	for (i = 0; i < len; i++)
	{
		buf[cnt++] = data[i];
	}
	
	checkSum = CheckSum((unsigned char*)buf, len + 3);
	buf[cnt++] = checkSum;
	buf[cnt++] = 0x23;
	
	Uart3_sendData(buf, len + 5);
	
}

void BLE_GetInfo(const char *str, char *out, char outLen)
{
	char send[32] = {0};
	char ack[32] = {0};
	
	if(str == NULL || out == NULL)
	{
		return;
	}
	
	snprintf(send, sizeof(send)-1, "AT+%s?", str);
	snprintf(ack, sizeof(ack)-1, "OK+%s:", str);
	
	if(BLE_sendAT(send, ack, 100) == 0)
	{
		Delay_ms(50);//再接收数据
		memcpy(out, &gBLE.rxBuf[strlen(ack)], outLen);
	}
	
	//特殊处理
	if (strcmp(str, "VERS") == 0)
	{
		memcpy(out, gBLE.rxBuf, outLen);
	}
}

int BLE_GetConnectStatus(void)
{
	if (gBLE.connectFlag == 0)
	{
		if (strstr((char*)gBLE.rxBuf, "OK+CONN:") != NULL)
		{
			Delay_ms(50);   //把连接的主机地址接收完全
			gBLE.connectFlag = 1;
			return 1;
		}
	}
	else
	{
		if (strstr((char*)gBLE.rxBuf, "OK+LOST") != NULL)
		{
			gBLE.connectFlag = 0;
			return 2;
		}
	}
	return 0;
}





#include "bluetooth.h"
#include "SysTick.h"
#include "string.h"
#include "stdio.h"
#include "print.h"
#include "flash_ee.h"


BLE_INFO gBLE;
BLE_BIND_INFO gBindInfo;   //蓝牙绑定信息

//蓝牙初始化
int BLE_Init(void)
{
	int isNeedRestart = 0;
	
	Uart3_Init(9600);
	gBLE.connectFlag = 0;   //蓝牙未连接
	
	if(BLE_sendAT("AT", "OK", 500) != 0)
	{
		Delay_ms(100);
		if(BLE_sendAT("AT", "OK", 500) != 0)
		{
			gBLE.isUartOk = 0;
			return -1;     //初始化失败，模块连接测试失败
		}
	}
	else
	{
		gBLE.isUartOk = 1;  //蓝牙模块连接测试OK
		PRINT("ble uart connect test ok \r\n");
	}
	
	//获取mac
	BLE_GetInfo("ADDR", gBLE.mac, sizeof(gBLE.mac));
	PRINT("ble mac: %s\r\n", gBLE.mac);
	
	//获取name 
	BLE_GetInfo("NAME", gBLE.name, sizeof(gBLE.name));
	PRINT("ble name: %s\r\n", gBLE.name);
	if (strcmp(gBLE.name, STATIONNAME) != 0)
	{
		isNeedRestart = 1;
		BLE_sendAT("AT+NAME"STATIONNAME, "...", 100);  //设置蓝牙节点名称
		PRINT("set ble name: %s\r\n", STATIONNAME);
	}
	
	//获取版本 
	BLE_GetInfo("VERS", gBLE.version, sizeof(gBLE.version));
	PRINT("ble version: %s\r\n", gBLE.version);
	
	BLE_sendAT("AT+TYPE?", "...", 100);
	PRINT("ble connectType:%c %s\r\n", gBLE.rxData.atConnectType.type[0], gBLE.rxData.atConnectType.type[0] == '2'?"connection need password":"");
	if (gBLE.rxData.atConnectType.type[0] != '3')
	{
		BLE_sendAT("AT+TYPE3", "...", 100);
		PRINT("set ble connectType 3, need password\r\n");
	}

	BLE_sendAT("AT+PASS?", "...", 100);
	PRINT("ble password: %s\r\n", gBLE.rxData.atPassword.password);
	memcpy(gBLE.password, gBLE.rxData.atPassword.password, 6);
	if (strcmp(gBLE.password, "000000") == 0)
	{
		PRINT("ble password is original password :%s!!!\r\n", ORIGINALPASSWORD);
		//BLE_sendAT("AT+PASS"STATIONPASS, "...", 100);
	}
	
	BLE_sendAT("AT+NOTI1", "...", 200);  //设置通知上位机连接状态
	BLE_sendAT("AT+NOTP1", "...", 200);  //连接成功后模块会发送，”OK+CONN:001122334455”字符
	Delay_ms(1);
	
	if (isNeedRestart)
	{
		BLE_sendAT("AT+ROLE0", "...", 100);            //设置为主模式，500ms后会重启
		Delay_ms(1000);                                //等待重启
		if(BLE_sendAT("AT", "OK", 500) != 0)
		{
			Delay_ms(100);
			if(BLE_sendAT("AT", "OK", 500) != 0)
			{
				PRINT("ble restart error!\r\n");
				return -1;
			}
		}
		PRINT("ble restart ok!\r\n");
	}
	
	gBLE.connectFlag = 0;   //蓝牙未连接
	BLE_Clear();
	return 0;
}


//==========================================================================================
//蓝牙初始化密码
int BLE_InitPassword(void)
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
	BLE_sendAT("AT+TYPE3", "...", 100);            //配连接需要密码
	BLE_sendAT("AT+PASS"ORIGINALPASSWORD, "...", 100);  //配置初始密码
	memcpy(gBLE.password, ORIGINALPASSWORD, PASSWORD_LEN);
//	BLE_sendAT("AT+RENEW", "...", 100);            //恢复出厂设置
	BLE_sendAT("AT+ALLO0", "...", 100);            //白名单功能关
	BLE_sendAT("AT+ROLE0", "...", 100);            //设置为主模式，500ms后会重启
	PRINT("蓝牙重启...\r\n");
	Delay_ms(1000);                                //等待重启
	
	if(BLE_sendAT("AT", "OK", 500) != 0)
	{
		Delay_ms(100);
		if(BLE_sendAT("AT", "OK", 500) != 0)
		{
			PRINT("ble restart error!\r\n");
			return -1;
		}
	}
	PRINT("ble restart ok!\r\n");
	
	BLE_Clear();
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
        gBLE.rxData.buf[i] = 0;
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
			pfind = strstr((char*)gBLE.rxData.buf, searchStr);
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
		memcpy(out, &gBLE.rxData.buf[strlen(ack)], outLen);
	}
	
	//特殊处理
	if (strcmp(str, "VERS") == 0)
	{
		Delay_ms(50);
		memcpy(out, gBLE.rxData.buf, outLen);
	}
}

int BLE_GetConnectStatus(void)
{
	if (gBLE.connectFlag == 0)
	{
		if (strstr((char*)gBLE.rxData.buf, "OK+CONN:") != NULL)
		{
			Delay_ms(50);   //把连接的主机地址接收完全
			gBLE.connectFlag = 1;
			return 1;
		}
	}
	else
	{
		if (strstr((char*)gBLE.rxData.buf, "OK+LOST") != NULL)
		{
			gBLE.connectFlag = 0;
			return 2;
		}
	}
	return 0;
}

void BLE_WriteBindInfo(void)
{
	Flash_write(BindInfoStartAddr, (uint32_t*)&gBindInfo, sizeof(gBindInfo) / 4);
}

void BLE_ReadBindInfo(void)
{
	Flash_read(BindInfoStartAddr, (uint32_t*)&gBindInfo, sizeof(gBindInfo) / 4);
	if(gBindInfo.chechSum != CheckSum((unsigned char*)&gBindInfo, sizeof(gBindInfo) - 4))
	{
		gBindInfo.isBinded = 0;
		gBindInfo.count = 0;
		memset(gBindInfo.bindMac, 0, sizeof(gBindInfo.bindMac));
		gBindInfo.chechSum = 0;
	}
}

//绑定mac
int BLE_Bind(char* mac)
{
	char str[32] = {0};
	uint32_t i;
	
	if (gBindInfo.count >= 3)
	{
		return 1;  //绑定mac已经3个了
	}
	
	
	for (i = 0; i < 3; i++)
	{
		if (memcmp(gBindInfo.bindMac[i], mac, 12) == 0)
		{
			return 2; //已经绑定过了
		}
	}
	

	memcpy(gBindInfo.bindMac[gBindInfo.count], mac, 12);

	if(BLE_sendAT("AT", "OK", 500) != 0)
	{
		PRINT("AT+AD error\r\n");
	}
	Delay_ms(200);  
	
	BLE_sendAT("AT+ALLO0", "...", 100);            //白名单功能开
	sprintf(str, "AT+AD%d%s", gBindInfo.count+1, gBindInfo.bindMac[gBindInfo.count]);
	if (BLE_sendAT(str, "OK+AD", 100) == 0)
	{
		Delay_ms(15);
		gBindInfo.isBinded = 1;
		gBindInfo.count++;
		gBindInfo.chechSum = CheckSum((unsigned char*)&gBindInfo, sizeof(gBindInfo) - 4);
		BLE_WriteBindInfo();
	}
	else
	{
		memset(gBindInfo.bindMac[gBindInfo.count], 0, 12);
		return 3;
	}
	
	return 0;
}

int BLE_BindClean(void)
{
	Delay_ms(100); 
	if(BLE_sendAT("AT", "OK", 500) != 0)
	{
		Delay_ms(100); 
		if(BLE_sendAT("AT", "OK", 500) != 0)
		{
			PRINT("bletooth clean error!\r\n");
			return -1;
		}
	}

//	BLE_sendAT("AT+RENEW", "...", 100);            //恢复出厂设置
	BLE_sendAT("AT+ROLE0", "...", 100);            //设置为从模式，500ms后会重启
	
	memset((char*)&gBindInfo, 0xff, sizeof(gBindInfo));
	BLE_WriteBindInfo();
	
	return 0;
}

int BLE_SetPassword(char *pass)
{
	char pass_temp[32];
	if (pass == NULL)
	{
		return -1;
	}

	snprintf(pass_temp, sizeof("AT+PASS") + 6, "AT+PASS%s", pass);
	PRINT("pass_temp:%s\r\n", pass_temp);
	
	Delay_ms(200);
	if(BLE_sendAT(pass_temp, "OK", 200) == 0)
	{
		Delay_ms(20);
		memcpy(gBLE.password, pass, PASSWORD_LEN);
		return 0;
	}
	else
	{
		return -1;
	}
}

void print_bleRx(void)
{
	uint32_t i = 0;
	for (i = 0; i < 24; i++)
	{
		printf("%02x ", gBLE.rxData.buf[i]);
	}
	printf("\r\n");
}

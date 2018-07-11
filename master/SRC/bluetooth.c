#include "bluetooth.h"
#include "SysTick.h"
#include "string.h"


BLE_COM g_bleCom;


void BLE_clear(void);


int BLE_Init(void)
{
	Uart3_Init(9600);
	
	BLE_sendAT("AT", "...", 100);
	BLE_sendAT("AT+IMME?", "...", 200);
	BLE_sendAT("AT+ROLE?", "...", 200);
//	BLE_sendAT("AT+ROLE0", "...", 200); //设置为主模式，500ms后会重启
//	Delay_ms(1000);
	
	BLE_clear();
	return 0;
}


void BLE_clear(void)
{
    u32 i;
    g_bleCom.rxCount = 0;			//接收计数器清零
	g_bleCom.rxTime = 0;
    for(i = 0; i < BLE_COM_RX_CNT_MAX; i++)
    {
        g_bleCom.rxBuf[i] = 0;
    }
}

int BLE_sendAT(char *sendStr, char *searchStr, u32 outTime)
{
    int ret = 0;
    char *pfind = 0;
	
	BLE_clear();
	
	Uart3_sendStr(sendStr);
	
    if(searchStr && outTime)//当searchStr和outTime不为0时才等待应答
    {
        while((--outTime)&&(pfind == 0))//等待指定的应答或超时时间到来
        {
			pfind = strstr((char*)g_bleCom.rxBuf, searchStr);
			if(pfind != 0)
			{
				break;
			}
			Delay_ms(1);
			g_bleCom.rxTime++;
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
	Delay_ms(5);
    return ret;
}

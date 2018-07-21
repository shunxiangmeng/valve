#include <stdio.h>
#include <string.h>
#include "SysTick.h"
#include "bluetooth.h"
#include "print.h"
#include "led.h"
#include "buzzer.h"
#include "charge.h"
#include "ir.h"
#include "time.h"

extern BLE_BIND_INFO gBindInfo;   //蓝牙绑定信息
extern BLE_INFO gBLE;

char cmd[7] = {0x01,'A', 'B', 'C', 'D', 'E', 0x00};

int main(void)
{
	uint8_t bleConnectStatus = 0;
	uint32_t timeOutCount = 0;
	int operated = -1;
	int ret = 0;
	
	Time_Init();
	Print_Init();    //初始化调试串口
	LED_Init();      //初始化LED
	BUZZER_Init();   //初始化蜂鸣器
	Charge_Init();   //初始化充电模块
	IR_Init();       //初始化红外通行口
	
	PRINT("........start system......\r\n");
	BUZZER_Set(1);
	LED_Set(WHITE);
	Delay_ms(40);
	BUZZER_Set(0);
	LED_Set(BLACK);
	
	//上电就进入绑定功能
	LED_Set(RED);
	Charge_On();  //打开充电开关
	PRINT("charge power on\r\n");
	Delay_ms(100);
	BLE_ReadBindInfo();             //读取flash中绑定的信息
	timeOutCount = 0;
	while(1)
	{
		if(gIR.rxFlag)              //收到阀门信号
		{
			if (gIR.rxCmd == 0x0A)  //communication start cmd
			{
				if (IR_GetValveID(1000) == 0)
				{
					PRINT("阀门ID:%s \r\n", gIR.valveID);
					if (strcmp(gIR.valveID, BIND_VALVE_ID) == 0)
					{
						PRINT("设置白名单  \r\n");
						goto NEED_BIND;
					}
					else if (memcmp(gIR.valveID, BIND_CLEAN_VALVE_ID, 6) == 0)
					{
						BLE_Init();
						BLE_BindClean();
						PRINT("删除白名单 \r\n");
						PRINT("系统重启!!!\r\n");
						BUZZER_start(100, 100, 10); //蜂鸣器响
						Delay_ms(2000);
						BUZZER_stop(); 
						BLE_Clear();
						Delay_ms(2);
						//重启
						__set_FAULTMASK(1);
						NVIC_SystemReset();
						while(1);
					}
					else
					{
						PRINT("不是绑定阀门，进入设定模式\r\n");
						break;
					}
				}
			}
		}
		
		Delay_ms(1);
		timeOutCount++;
		if (gBindInfo.isBinded != 0 && timeOutCount > 4000) //已经绑定过且超时，则进入正常程序
		{
			break;
		}	
		if (timeOutCount % 1000 == 0)
		{
			PRINT("wait valve connect:%d\r\n",timeOutCount);
		}
	}

	BLE_Init();
NORMAL:	
	Charge_Off();
	LED_Set(0);
	PRINT("valve power off\r\n");
	PRINT("白名单1: %s\r\n", gBLE.AdMac1);
	PRINT("白名单2: %s\r\n", gBLE.AdMac2);
	PRINT("白名单3: %s\r\n", gBLE.AdMac3);
	PRINT("进入设定模式\r\n");
	while(1)
	{
		Delay_ms(1);
		bleConnectStatus = BLE_GetConnectStatus();
		if(bleConnectStatus == 1) //connect
		{
			PRINT("bluetooth connect form mac:%s\r\n", &gBLE.rxBuf[sizeof("OK+CONN:")-1]);
			memcpy(gBLE.conenctMac, &gBLE.rxBuf[sizeof("OK+CONN:")-1], 12);
			Delay_ms(1);
			BLE_Clear();
		}
		else if (bleConnectStatus == 2) //lost
		{
			PRINT("bluetooth disconnect!\r\n");
			Delay_ms(1);
			BLE_Clear();
		}
		
		if (gBLE.rxFlag)
		{
			switch(gBLE.rxCmd)
			{
				case 0x1A:  //获取阀门编号
					PRINT("handle 0X1A cmd\r\n");
					BLE_Clear();
					IR_clear();
					Charge_On();
					LED_Set(RED);
					PRINT("valve power on, wait valve connect...\r\n");
					timeOutCount = 0;
					while(1)
					{
						Delay_ms(1);
						if(gIR.rxFlag)
						{
							switch(gIR.rxCmd)
							{
								case 0x0A:  //请求连接
									gIR.rxCmd = 0;
									operated = 0;
									if (IR_GetValveIDAndOpen(gBLE.rxPassword, 1000) == 0)
									{
										PRINT("got valave id :%s!\r\n", gIR.valveID);
										memcpy(&cmd[1], gIR.valveID, 6);
										operated = gIR.rxBuf[11];
										IR_SendAck(gIR.randomCode, 0);
										BLE_SendData(0x08, cmd, 7);
										PRINT("operated:%d\r\n", operated);
									}
									else
									{
										PRINT("get valve id time out \r\n");
									}
									break;
								case 0x02:
									IR_SendAck(gIR.randomCode, 0);
									break;
								default:break;
							}
						}
						timeOutCount++;
						if (timeOutCount > 30*1000)
						{
							PRINT("timeOut!\r\n");
							break;
						}
						if (operated == 0)
						{
							PRINT("获取阀门ID成功\r\n");
							operated = -1;
							Charge_Off();  
							break;
						}
						else if(operated == 2)
						{
							PRINT("密码错误\r\n");
							operated = -1;
							Charge_Off();  
							break;
						}
					}
					break;
					
				case 0x3A:  //钢瓶有效使用日期和气站密码
					PRINT("设置钢瓶有效使用日期:%s\r\n", gBLE.date);
					BLE_Clear();
					Charge_On();
					timeOutCount = 0;
					while(1)
					{
						Delay_ms(1);
						if(gIR.rxFlag)
						{
							switch(gIR.rxCmd)
							{
								case 0x0A:  //请求连接
									operated = 0;
									if (IR_WriteValveFlaskTime(gBLE.rxPassword, gIR.valveID, gBLE.date, 1000) == 0)
									{
										PRINT("写入有效日期ret:%d\r\n",0);
										operated = 0;
										IR_SendAck(gIR.randomCode, 0);
										PRINT("operated:%d\r\n", operated);
										BLE_SendData(0x3A, cmd, 1); 
										BLE_Clear();										
									}
									break;
								case 0x08:
									break;
							}
						}
						timeOutCount++;
						if (timeOutCount > 10*1000)
						{
							PRINT("timeOut!\r\n");
							break;
						}
						if (operated == 0)
						{
							PRINT("写入有效日期成功\r\n");
							operated = -1;
							Charge_Off();  
							break;
						}
						else if(operated == 2)
						{
							PRINT("密码错误\r\n");
							operated = -1;
							Charge_Off();  
							break;
						}
					}
					break;
				default:break;
			}
		}
	}
	
	
//====================================================================================
//绑定流程	
NEED_BIND:
	LED_Set(BLUE);
	PRINT("开始添加白名单\r\n");
	if (BLE_BindInit() != 0)
	{
		PRINT("bluetooth uart init failed!\r\n");
		while(1)
		{
			LED_Set(WHITE);
			Delay_ms(100);
		}
	}
	LED_Set(BLUE);
	BUZZER_start(500, 500, 60); //蜂鸣器响
	LED_SetFlash(500, 500, 100, GREEN);
	PRINT("等待蓝牙连接\r\n");
	while(1)
	{
		Delay_ms(1);
		bleConnectStatus = BLE_GetConnectStatus();
		if(bleConnectStatus == 1) //connect
		{
			PRINT("蓝牙连接成功\r\n");
			ret = BLE_Bind((char*)&gBLE.rxBuf[sizeof("OK+CONN:") - 1]);
			if ( ret == 0)
			{
				PRINT("蓝牙成功绑定手机地址MAC[%d]:%s\r\n", gBindInfo.count-1, gBindInfo.bindMac[gBindInfo.count - 1]);
				BUZZER_stop(); 
				BLE_Clear();
				PRINT("系统重启\r\n");
				Delay_ms(100);
				//重启
				__set_FAULTMASK(1);
				NVIC_SystemReset();
				while(1);
			}
			else if (ret == 1)
			{
				PRINT("绑定地址已达3个\r\n");
			}
			else if (ret == 2)
			{
				PRINT("该地址%s已绑定！\r\n", (char*)&gBLE.rxBuf[sizeof("OK+CONN:") - 1]);
			}

		}
		timeOutCount++;
		if (timeOutCount > 30*1000)
		{
			PRINT("等待蓝牙连接超时\r\n");
			BUZZER_stop(); 
			goto NORMAL;
		}
	}
}

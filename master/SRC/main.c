#include <stdio.h>
#include <string.h>
#include "SysTick.h"
#include "bluetooth.h"
#include "usart2.h"
#include "led.h"
#include "buzzer.h"
#include "charge.h"
#include "ir.h"
#include "flash_ee.h"
#include "time.h"

extern SYSTEM_INFO gSysInfo;

uint8_t password[7] = {"654321"};
uint8_t count = 0;
char cmd[7] = {0x01,'A', 'B', 'C', 'D', 'E', 0x00};

int main(void)
{
	uint8_t bindValveConnectFlag = 0;
	uint8_t bleConnectStatus = 0;
	uint32_t timeOutCount = 0;
	uint32_t operated = 0;
	
	Time_Init();
	Print_Init();    //初始化调试串口
	LED_Init();      //初始化LED
	BUZZER_Init();   //初始化蜂鸣器
	Charge_Init();   //初始化充电模块
	IR_Init();       //初始化红外通行口
	
	PRINT("start system......\r\n");
	BUZZER_Set(1);
	LED_Set(WHITE);
	Delay_ms(40);
	BUZZER_Set(0);
	LED_Set(BLACK);
	
	SysInfo_read();  //读取flash中的信息
	if (!gSysInfo.isBleBind)  //气站蓝牙没有绑定过，则进行绑定
	{
		printf("bluetooth didn't bind, start bind......\r\n");
		if (BLE_BindInit() != 0)
		{
			printf("bluetooth init failed!\r\n");
			while(1)
			{
				Delay_ms(100);
			}
		}
		//BLE_RxStatus(DISABLE);  //关闭蓝牙的接收
		
		printf("IR power on\r\n");
		Charge_On();  //打开充电开关
		LED_Set(BLUE);
		BUZZER_start(500, 500, 10);
		while(1)
		{
			Delay_ms(1);
			if(gIR.rxFlag)
			{
				switch(gIR.rxCmd)
				{
					case 0x0A:  //communication start cmd
						if (IR_GetValveID(1000) == 0)
						{
							if (strcmp(gIR.valveID, BIND_VALVE_ID) == 0)
							{
								//绑定阀门连接成功
								bindValveConnectFlag = 1;
								printf("bind valve connected!\r\n");
							}
							Delay_ms(1);
						}
						break;
				}
			}
			if (bindValveConnectFlag)
			{
				break;
			}
		}
		//等待蓝牙连接
		printf("等待蓝牙连接\r\n");
		//BLE_RxStatus(ENABLE);  //打开蓝牙的接收
		while(1)
		{
			Delay_ms(1);
			bleConnectStatus = BLE_GetConnectStatus();
			if(bleConnectStatus == 1) //connect
			{
				printf("蓝牙连接成功\r\n");
				printf("蓝牙绑定MAC:%s\r\n", &gBLE.rxBuf[sizeof("OK+CONN:") - 1]);
				BLE_Clear();
				gSysInfo.isBleBind = 1;
				gSysInfo.chechSum = CheckSum((unsigned char*)&gSysInfo, sizeof(gSysInfo) - 4);
				SysInfo_write();  //写flash信息
				
				printf("reboot system......\r\n");
				Delay_ms(100);
				//重启
				__set_FAULTMASK(1);
				NVIC_SystemReset();
				while(1);
			}
		}
	}
	

	Charge_Off();
//	LED_Set(0);
	
	PRINT("hello,system start...\r\n");
	//BUZZER_start(300, 700, 5);

	BLE_Init();
	
	LED_SetFlash(500, 500, 100, GREEN);
	
	while(1)
	{
		bleConnectStatus = BLE_GetConnectStatus();
		if(bleConnectStatus == 1) //connect
		{
			PRINT("bluetooth connect\r\n");
			Delay_ms(1);
			BLE_Clear();
		}
		else if (bleConnectStatus == 2) //lost
		{
			PRINT("bluetooth disconnect\r\n");
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
								case 0x0A:  //获取阀门编号
									gIR.rxCmd = 0;
									operated = 0;
									if (IR_GetValveID(500) == 0)
									{
										PRINT("got valave id :%s!\r\n", gIR.valveID);
										memcpy(&cmd[1], gIR.valveID, 6);
										IR_SendAck(gIR.randomCode, 0);
										BLE_SendData(0x1A, cmd, 7);
										operated = 1;
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
						if (operated)
						{
							PRINT("获取阀门ID成功\r\n");
							operated = 0;
							break;
						}
						if (timeOutCount > 30*1000)
						{
							PRINT("timeOut!\r\n");
							break;
						}
					}
					Charge_Off();  
					break;
				case 0x2A:  //���ű�Ż�ȡ���֪ͨ
					BLE_SendData(0x2A, cmd, 1);
					BLE_Clear();
					break;
				case 0x3A:  //��ƿ��Чʹ�����ں���վ����
					BLE_SendData(0x3A, cmd, 1);  //Ӧ��ɹ�
					BLE_Clear();
					break;
				default:break;
			}
		}
		
		
		/*
		Delay_ms(1);
		if(gIR.rxFlag)
		{
			switch(gIR.rxCmd)
			{
				case 0x0A:
					printf("IR recv cmd 0x0A\r\n");
					count++;
					IR_SendCMD(0x02, gIR.randomCode, password, 6);
					break;
				case 0x02:
					IR_SendAck(gIR.randomCode, 0);
					break;
				default:break;
			}
		}
		*/
	}
}

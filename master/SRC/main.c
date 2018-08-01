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

extern BLE_INFO gBLE;

BLE_TX gBleSend;

//临时变量
struct
{
	char password[6];
	char id[6];
	char date[6];
}g_valveTemp;

int main(void)
{
	uint8_t bleConnectStatus = 0;
	int ret = -1;
	
	Time_Init();
	Print_Init();    //初始化调试串口
	LED_Init();      //初始化LED
	BUZZER_Init();   //初始化蜂鸣器
	Charge_Init();   //初始化充电模块
	IR_Init();       //初始化红外通行口
	
	printf("\r\n");
	PRINT("........start system......\r\n");
	BUZZER_Set(1);
	LED_Set(WHITE);
	Delay_ms(40);
	BUZZER_Set(0);
	LED_Set(BLACK);
	BLE_Init();
	
	//启动检测是否需要密码还原
	LED_Set(RED);
	Charge_On();  //打开充电开关
	PRINT("wait vlave connect...\r\n");
	if(IR_WaitConnect(5000) == 0)
	{
		PRINT("valve connect for reset bluetooth password\r\n");
		if (IR_GetValveID(1000) == 0)
		{
			PRINT("valve ID:%s\r\n", gIR.valveID);
			if (strcmp(gIR.valveID, BIND_VALVE_ID) == 0)
			{
				//初始密码
				PRINT("init bluetooth password\r\n");
				BLE_SetPassword(ORIGINALPASSWORD);
				BUZZER_start(100, 100, 10); //蜂鸣器响
				Delay_ms(2000);
			}
		}
	}
	Charge_Off();

	PRINT("start work......\r\n");
	LED_Set(YELLOW);
	LED_SetFlash(500, 500, 1000, GREEN);
	
	while(1)
	{
		bleConnectStatus = BLE_GetConnectStatus();
		if(bleConnectStatus == 1) //connect
		{
			PRINT("bluetooth connect form mac:%s\r\n", &gBLE.rxData.buf[sizeof("OK+CONN:")-1]);
			memcpy(gBLE.conenctMac, &gBLE.rxData.buf[sizeof("OK+CONN:")-1], 12);
			Delay_ms(1);
			BLE_Clear();
			PRINT("the bluetooth password is:%s\r\n", gBLE.password);
			if (memcmp(gBLE.password, ORIGINALPASSWORD, PASSWORD_LEN) == 0)
			{
				PRINT("the bluetooth password is original password, need change\r\n");
				memcpy(gBleSend.buf, ORIGINALPASSWORD, PASSWORD_LEN);
				BLE_SendData(0x4A, gBleSend.buf, PASSWORD_LEN);   //重置密码
			}
			LED_SetFlash(500, 500, 0, GREEN);
			LED_Set(WHITE);
		}
		else if (bleConnectStatus == 2) //lost
		{
			PRINT("bluetooth disconnect!\r\n");
			Delay_ms(1);
			BLE_Clear();
			LED_Set(0);
			LED_SetFlash(500, 500, 1000, GREEN);
		}
		
		if (gBLE.rxFlag)
		{
			gBLE.rxFlag = 0;
			PRINT("BLE CMD: 0x%02X \r\n", gBLE.rxCmd);
			switch(gBLE.rxCmd)
			{
				case 0x1A: 
					memcpy(g_valveTemp.password, gBLE.rxData.cmdGetValveId.password, PASSWORD_LEN);  //临时变量保存
					if (memcmp(gBLE.password, ORIGINALPASSWORD, PASSWORD_LEN) == 0)
					{
						PRINT("the bluetooth password is original password, need change\r\n");
						memcpy(gBleSend.buf, ORIGINALPASSWORD, PASSWORD_LEN);
						BLE_SendData(0x4A, gBleSend.buf, 6);   //重置密码
					}
					else
					{
						IR_clear();
						Charge_On();
						if(IR_WaitConnect(30000) == 0)
						{
							PRINT("valve connect success\r\n");
							Delay_ms(100);
							if (IR_GetValveIDAndOpen(g_valveTemp.password, 1000) == 0)
							{
								print_irRx();
								PRINT("got valave id: %s \r\n", gIR.valveID);
								ret = gIR.rxBuf[11];
								gBleSend.sendValveId.ret = ret;
								IR_SendAck(gIR.randomCode, 0);
								PRINT("ret: %d\r\n", ret);
								if (ret == 0)
								{
									memcpy(gBleSend.sendValveId.valveId, gIR.valveID, 6);
									BLE_SendData(0x1A, gBleSend.buf, sizeof(gBleSend.sendValveId));   //返回ID
								}
								else
								{
									memcpy(gBleSend.sendValveId.valveId, "000000", 6);
									BLE_SendData(0x1A, gBleSend.buf, sizeof(gBleSend.sendValveId));   //返回错误的ID
								}
							}
							else
							{
								gBleSend.sendValveId.ret = 1;
								memcpy(gBleSend.sendValveId.valveId, "000000", 6);
								BLE_SendData(0x1A, gBleSend.buf, sizeof(gBleSend.sendValveId));   //返回ID
							}
						}
						else
						{
							PRINT("waite valve connect failed!\r\n");
							gBleSend.sendValveId.ret = 1;
							memcpy(gBleSend.sendValveId.valveId, "000000", 6);
							BLE_SendData(0x1A, gBleSend.buf, sizeof(gBleSend.sendValveId));   //返回ID
						}
						Charge_Off();
					}
					break;
					
					//------------------------------------------------------------
				case 0x3A:  //写入钢瓶有效使用日期
					memcpy(g_valveTemp.password, gBLE.rxData.cmdWriteDate.passwrod, PASSWORD_LEN);  //临时变量保存
					memcpy(g_valveTemp.id, gBLE.rxData.cmdWriteDate.valveId, 6);  //临时变量保存
					memcpy(g_valveTemp.date, gBLE.rxData.cmdWriteDate.date, 6);  //临时变量保存
					if (memcmp(gBLE.password, ORIGINALPASSWORD, PASSWORD_LEN) == 0)
					{
						PRINT("the bluetooth password is original password, need change\r\n");
						memcpy(gBleSend.buf, ORIGINALPASSWORD, PASSWORD_LEN);
						BLE_SendData(0x4A, gBleSend.buf, PASSWORD_LEN);   //重置密码
					}
					else
					{
						IR_clear();
						Charge_On();
						PRINT("wait valve connect\r\n");
						if(IR_WaitConnect(30000) == 0)
						{
							print_bleRx();
							PRINT("ps:%s, ID:%s\r\n", g_valveTemp.password, g_valveTemp.id);
							if (IR_WriteValveFlaskTime(g_valveTemp.password, g_valveTemp.id, g_valveTemp.date, 1000) == 0)
							{
								ret = gIR.rxBuf[11];
								IR_SendAck(gIR.randomCode, 0);
								PRINT("write valid date ret:%d\r\n", ret);
								gBleSend.buf[0] = ret;
								BLE_SendData(0x3A, gBleSend.buf, 1); 
								BLE_Clear();										
							}
							else
							{
								gBleSend.buf[0] = 1;
								BLE_SendData(0x3A, gBleSend.buf, 1);
							}
						}
						else
						{
							PRINT("connect failed\r\n");
							gBleSend.buf[0] = 1;
							BLE_SendData(0x3A, gBleSend.buf, 1);
						}
						Charge_Off();
					}
					break;
					
				//-------------------------------
				//设置密码
				case 0x5A:
					PRINT("set bluetooth password\r\n");
					PRINT("%s %s\r\n",gBLE.password, gBLE.rxData.cmdSetBlePassword.password_old);
					if (memcmp(gBLE.password, gBLE.rxData.cmdSetBlePassword.password_old, 6) == 0)
					{
						PRINT("new %s \r\n",gBLE.rxData.cmdSetBlePassword.password_new);
						
						gBleSend.buf[0] = 0;
						memcpy(&gBleSend.buf[1], gBLE.rxData.cmdSetBlePassword.password_new, 6);
						BLE_SendData(0x5A, gBleSend.buf, 7);
						Delay_ms(100);
						BLE_sendAT("AT+ERASE", "OK+ERASE", 100); 
						Delay_ms(100);
						BLE_sendAT("AT", "OK+LOST", 100);  //断开连接
						LED_Set(0);
						LED_SetFlash(500, 500, 1000, GREEN);
						if (BLE_SetPassword(&gBleSend.buf[1]) == 0)
						{
							PRINT("set password success\r\n");
							//BLE_sendAT("AT", "OK", 100);  //断开连接
						}
						else
						{
							PRINT("set password error\r\n");
						}
					}
					else
					{
						gBleSend.buf[0] = 1;
						BLE_SendData(0x5A, gBleSend.buf, 1);
					}
					break;
				default: break;
			}
				
		}
	}
	

}

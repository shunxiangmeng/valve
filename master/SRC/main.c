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
extern STATION_PASSWORD_INFO gPasswordInfo; 

BLE_TX gBleSend;

//临时变量
struct
{
	char password[6];
	char id[6];
	char date[6];
}g_valveTemp;

//================================================================================
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
	PRINT("........system start......\r\n");
	BUZZER_Set(1);
	LED_Set(WHITE);
	Delay_ms(40);
	BUZZER_Set(0);
	LED_Set(BLACK);
	BLE_Init();
	BLE_ReadSationPasswordInfo();
	
	//启动检测是否需要密码还原
	LED_Set(RED);
	Charge_On();  //打开充电开关
	PRINT("wait vlave connect for reset password...\r\n");
	if(IR_WaitConnect(5000) == 0)
	{
		PRINT("valve connect for reset bluetooth password\r\n");
		if (IR_GetValveID(1000) == 0)
		{
			PRINT("valve ID:%s\r\n", gIR.valveID);
			if (strcmp(gIR.valveID, RESET_PASSWORD_VALVE_ID) == 0)
			{
				//初始密码
				PRINT("init station password\r\n");
				BLE_SetStationPassword(ORIGINALPASSWORD);
				BUZZER_start(100, 100, 10); //蜂鸣器响
				Delay_ms(2000);
			}
		}
	}
	Charge_Off();

	PRINT("start work......\r\n");
	LED_Set(YELLOW);
	LED_SetFlash(500, 500, 100000, GREEN);
	
	while(1)
	{
		bleConnectStatus = BLE_GetConnectStatus();
		if(bleConnectStatus == 1) //connect
		{
			PRINT("bluetooth connect form mac:%s\r\n", &gBLE.rxData.buf[sizeof("OK+CONN:")-1]);
			memcpy(gBLE.conenctMac, &gBLE.rxData.buf[sizeof("OK+CONN:")-1], 12);
			Delay_ms(1);
			
			PRINT("station password is:%s\r\n", gBLE.password);
			//请求登陆
			Delay_ms(1500);
			BLE_Clear();
			gBleSend.buf[0] = 0;
			BLE_SendData(0x6A, gBleSend.buf, 1); 
			
			LED_SetFlash(500, 500, 0, GREEN);
			LED_Set(WHITE);
		}
		else if (bleConnectStatus == 2) //lost
		{
			PRINT("bluetooth disconnect!\r\n");
			gBLE.isLogin = FALSE;
			Delay_ms(1);
			BLE_Clear();
			LED_Set(0);
			LED_SetFlash(500, 500, 100000, GREEN);
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
						PRINT("wait valve connect\r\n");
						if(IR_WaitConnect(30000) == 0)
						{
							PRINT("valve connect success\r\n");
							Delay_ms(100);
							if (IR_GetValveIDAndOpen(g_valveTemp.password, 1000) == 0)
							{
								//print_irRx();
								PRINT("got valave id: %s \r\n", gIR.valveID);
								ret = gIR.rxBuf[11];
								gBleSend.sendValveId.ret = ret;
								IR_SendAck(gIR.randomCode, 0);
								PRINT("ret: %d\r\n", ret);
								if (ret == 0)
								{
									memcpy(gBleSend.sendValveId.valveId, gIR.valveID, VALVEID_LEN);
									BLE_SendData(0x1A, gBleSend.buf, sizeof(gBleSend.sendValveId));   //返回ID
									BUZZER_start(100, 100, 2); //蜂鸣器响
								}
								else
								{
									memcpy(gBleSend.sendValveId.valveId, "000000", VALVEID_LEN);
									BLE_SendData(0x1A, gBleSend.buf, sizeof(gBleSend.sendValveId));   //返回错误的ID
								}
							}
							else
							{
								gBleSend.sendValveId.ret = 1;
								memcpy(gBleSend.sendValveId.valveId, "000000", VALVEID_LEN);
								BLE_SendData(0x1A, gBleSend.buf, sizeof(gBleSend.sendValveId));   //返回ID
							}
						}
						else
						{
							PRINT("wait valve connect timeout!\r\n");
							gBleSend.sendValveId.ret = 1;
							memcpy(gBleSend.sendValveId.valveId, "000000", VALVEID_LEN);
							BLE_SendData(0x1A, gBleSend.buf, sizeof(gBleSend.sendValveId));   //返回ID
						}
						Charge_Off();
					}
					break;
					
					//------------------------------------------------------------
				case 0x3A:  //写入钢瓶有效使用日期
					memcpy(g_valveTemp.password, gBLE.rxData.cmdWriteDate.passwrod, PASSWORD_LEN);  //临时变量保存
					memcpy(g_valveTemp.id, gBLE.rxData.cmdWriteDate.valveId, VALVEID_LEN);  //临时变量保存
					memcpy(g_valveTemp.date, gBLE.rxData.cmdWriteDate.date, DATE_LEN);  //临时变量保存
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
							//print_bleRx();
							PRINT("ps:%s, ID:%s\r\n", g_valveTemp.password, g_valveTemp.id);
							if (IR_WriteValveFlaskTime(g_valveTemp.password, g_valveTemp.id, g_valveTemp.date, 1000) == 0)
							{
								ret = gIR.rxBuf[11];
								IR_SendAck(gIR.randomCode, 0);
								PRINT("write valid date ret:%d\r\n", ret);
								gBleSend.buf[0] = ret;
								if (ret == 0)
								{
									BUZZER_start(100, 100, 2); //蜂鸣器响
								}
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
					PRINT("set station password\r\n");
					PRINT("old pass:%s %s\r\n",gBLE.password, gBLE.rxData.cmdSetBlePassword.password_old);
					if (memcmp(gBLE.password, gBLE.rxData.cmdSetBlePassword.password_old, PASSWORD_LEN) == 0)
					{
						PRINT("new stationPassword: %s \r\n",gBLE.rxData.cmdSetBlePassword.password_new);
						
						BLE_SetStationPassword(gBLE.rxData.cmdSetBlePassword.password_new);
						gBleSend.buf[0] = 0;
						BLE_SendData(0x5A, gBleSend.buf, 1);
						Delay_ms(100);

						Delay_ms(100);
						BLE_sendAT("AT", "OK+LOST", 100);  //断开连接
						LED_Set(0);
						LED_SetFlash(500, 500, 1000, GREEN);

						PRINT("set password success\r\n");
						Delay_ms(500);
					}
					else
					{
						gBleSend.buf[0] = 1;
						BLE_SendData(0x5A, gBleSend.buf, 1);
					}
					break;
					
					//----------------------------------------------------------------------
				case 0x6A:
					if (memcmp(gBLE.rxData.cmdLogin.password, gPasswordInfo.stationPassword, PASSWORD_LEN) == 0)
					{
						PRINT("login succ\r\n");
						gBLE.isLogin = TRUE;
						
						gBleSend.buf[0] = 0;
						BLE_Clear();
						BLE_SendData(0x7A, gBleSend.buf, 1);
						
						if (memcmp(gBLE.password, ORIGINALPASSWORD, PASSWORD_LEN) == 0)
						{
							Delay_ms(1500);
							PRINT("the station password is original password, need change\r\n");
							gBleSend.buf[0] = 0;
							memcpy(&gBleSend.buf[1], ORIGINALPASSWORD, PASSWORD_LEN);
							BLE_Clear();
							BLE_SendData(0x4A, gBleSend.buf, PASSWORD_LEN+1);   //重置密码
						}
					}
					else
					{
						PRINT("login failed\r\n");
						gBleSend.buf[0] = 1;
						BLE_SendData(0x7A, gBleSend.buf, 1);
					}
					break;
				
				default: break;
			}
				
		}
	}//end while
}


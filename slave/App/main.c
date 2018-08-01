#include "stm32f0xx.h"
#include "stdio.h"
#include "string.h"
#include "led.h"
#include "delay.h"
#include "pwm.h"
#include "usart.h"
#include "flash_ee.h"
#include "Include.h"

//这个阀门是绑定阀门,如果不是，注释掉这个宏定义
#define BIND_VALVE  1

/* 宏定义 --------------------------------------------------------------------*/
#define BUF_SIZE                  128            //BUF长度
uint32_t WriteBuf[BUF_SIZE] = {0};
uint32_t ReadBuf[BUF_SIZE];

uint16_t RamdomCode = 0x1234;
uint8_t LastCmd = 0xff;
uint8_t CommResult = 0xff; //通信结果

unsigned char test433[8] = {0x40, 0xA5, 0x02, 0x02, 0x03, 0x04,0x05,0x06};


extern IRDA_COMM iUart;

struct SYS_DATA
{
  uint8_t Password[6];
  uint8_t ValveTime[6];
  uint8_t FlaskTime[6];
  uint8_t ValveId[6];
  uint8_t SuperPassword[12];
  uint8_t Cs;
}SysPraData;


//函数声明
void FLASH_TEST(void);


uint8_t Irda_SendNByte(uint8_t *data,uint8_t len);
void Save_Data_Pra(void);
void IRAD_Send_Start(void);
void IRAD_Send_Valve_Info(void);

void Irda_SendByte(uint8_t *data);

void Irda_Modulate(uint8_t state);

void Enable_SWD_Pin(void);
uint8_t isResetBind(void);

/************************************************
函数名称 ： System_Initializes
功    能 ： 系统初始化
参    数 ： 无
返 回 值 ： 无
作    者 ： strongerHuang
*************************************************/
void System_Initializes(void)
{
	Delay_Init(48);
	LED_Init();
    USART1_Init();
    TIM3_PWM_Init(1262);	
	//Timer3_PWM_SetDutyCycle(33); 
}

/************************************************
函数名称 ： Enable_SWD_Pin
功    能 ： 使能SWD引脚
参    数 ： 无
返 回 值 ： 无
作    者 ： strongerHuang
*************************************************/
void Enable_SWD_Pin(void)
{

          /* 使能GPIOA时钟 */
         //RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
         
         GPIO_PinAFConfig(GPIOA,GPIO_Pin_13,GPIO_AF_0);//GPIOA13复用为swd
         GPIO_PinAFConfig(GPIOA,GPIO_Pin_14,GPIO_AF_0);//GPIOA14复用为swd
         
}

/************************************************
函数名称 ： SoftwareDelay
功    能 ： 软件延时
参    数 ： Cnt --- 延时计数
返 回 值 ： 无
作    者 ： strongerHuang
*************************************************/
void SoftwareDelay(uint32_t Cnt)
{
  while(Cnt--);
}

/************************************************
函数名称 ： main
功    能 ： 主函数入口
参    数 ： 无
返 回 值 ： 无
作    者 ： strongerHuang
*************************************************/
int main(void)
{
  uint8_t SuccessFinishFlag = 0;
  uint8_t i;
  uint8_t *t = NULL;
  uint16_t iFlaskTime = 0;
  uint16_t iValveTime = 0;
  uint16_t iTimeTmp = 0;
  uint8_t ValveOpenFlag = 0;
	
  System_Initializes();
	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	//Enable_SWD_Pin();
	
  delay_ms(10);
	
  iUart.WaitFunc = &UART1_Wait;
  iUart.SendFunc = &Irda_SendNByte;
  iUart.ResetFunc = &UART1_Clear;
	
  for(i=0;i<2;i++)
  {
	delay_ms(100);
    LED_ON; 
    delay_ms(100);
    LED_OFF; 
  }
  
  flash_read(MyStartAddr,ReadBuf,sizeof(SysPraData));
  if(ReadBuf[sizeof(SysPraData)-1] == FlashCheckSum(ReadBuf, sizeof(SysPraData)-1))
  {
	  t = SysPraData.Password;
	  for(i=0; i < sizeof(SysPraData); i++)
	  {
		*t = (uint8_t)ReadBuf[i];
		 t++;
	  }
  }
  else
  {
	memset(SysPraData.Password, 0, sizeof(SysPraData));
	memset(SysPraData.Password, '0', sizeof(SysPraData.Password));  
  }
  
  	SysPraData.ValveId[0] = 'F';
	SysPraData.ValveId[1] = 'F';
	SysPraData.ValveId[2] = '0';
	SysPraData.ValveId[3] = 'F';
	SysPraData.ValveId[4] = 'F';
	SysPraData.ValveId[5] = 'A';
  
#ifdef BIND_VALVE 
  if (isResetBind())
  {
	//重置的ID
	SysPraData.ValveId[0] = 'F';
	SysPraData.ValveId[1] = 'F';
	SysPraData.ValveId[2] = 'F';
	SysPraData.ValveId[3] = 'F';
	SysPraData.ValveId[4] = 'F';
	SysPraData.ValveId[5] = 'B';
  }
#endif
  
  iFlaskTime = (SysPraData.FlaskTime[0]*10+SysPraData.FlaskTime[1])*12;
  iFlaskTime += SysPraData.FlaskTime[2]*10+SysPraData.FlaskTime[3];
  
  iValveTime = (SysPraData.ValveTime[0]*10+SysPraData.ValveTime[1])*12;
  iValveTime += SysPraData.ValveTime[2]*10+SysPraData.ValveTime[3];
  
  USART_Cmd(USART1, DISABLE);
	
  while(1)
  {
    while(SuccessFinishFlag == 1){}
    iUart.ResetFunc();
    IRAD_Send_Start();
    if(iUart.WaitFunc(1000) == 0)
    {
      LastCmd = iUart.Rxd.Buf[1];
      LED_ON;                                      //LED亮
      CommResult = 1;  //临时测试用
      switch (iUart.Rxd.Buf[1])
      {
		  case 0xFE:  //获取阀门编号，不开阀门，绑定蓝牙专用
			iUart.ResetFunc();
			IRAD_Send_Valve_Info();
			if(iUart.WaitFunc(1000) == 0 )
			{
				SuccessFinishFlag = 1;
			}
			break;
			
		  case 0x01://开阀（时间条件）
			if(memcmp(SysPraData.Password,iUart.Rxd.OpenValve.StationPassword,6) == 0 ||
				memcmp(SysPraData.SuperPassword,iUart.Rxd.SuperPassword.Data,12) == 0)
			{
				iTimeTmp = (iUart.Rxd.OpenValve.CurrentTtim[0]*10+iUart.Rxd.OpenValve.CurrentTtim[1])*12;
                iTimeTmp += iUart.Rxd.OpenValve.CurrentTtim[2]*10+iUart.Rxd.OpenValve.CurrentTtim[3];
                if((iTimeTmp >= iFlaskTime) && ((iTimeTmp - iValveTime) < 48))
                {
					CommResult = 0;
                }
			}
			iUart.ResetFunc(); 
			IRAD_Send_Valve_Info();
			
			if(iUart.WaitFunc(1000) == 0 )
			{
				ValveOpenFlag = 1;
			}
			break;
		
		  case 0x09:  //获取ID
			  if (memcmp(SysPraData.Password, "000000", 6) == 0) //初始密码
			  {
				  memcpy(SysPraData.Password, iUart.Rxd.GetValveID.StationPassword, 6); //设置密码
				  Save_Data_Pra();
				  iUart.ResetFunc(); 
				  CommResult = 0;   //操作成功
				  IRAD_Send_Valve_Info();
				  if(iUart.WaitFunc(1000) == 0 )
				  {
					ValveOpenFlag = 1;
				  } 
			  }
			  else if (memcmp(SysPraData.Password, iUart.Rxd.GetValveID.StationPassword, 6) == 0)  //匹配密码
			{
				iUart.ResetFunc(); 
				CommResult = 0;
				IRAD_Send_Valve_Info();
				if(iUart.WaitFunc(1000) == 0 )
				{
					ValveOpenFlag = 1;
				}
			}
			else  //密码匹配不上
			{
				iUart.ResetFunc(); 
				CommResult = 2; //密码错误
				IRAD_Send_Valve_Info();
				if(iUart.WaitFunc(1000) == 0 )
				{
				//	ValveOpenFlag = 1;
				}
			}
			break;
		case 0x02: //写入煤气站密码
			memcpy(SysPraData.Password,iUart.Rxd.Password.Data,6);
			Save_Data_Pra();
			iUart.ResetFunc(); 
			IRAD_Send_Valve_Info();
			if(iUart.WaitFunc(1000) == 0 )
			{
				SuccessFinishFlag = 1;
			}
			break;
		case 0x03: //写入阀门生产年月
			memcpy(SysPraData.ValveTime,iUart.Rxd.ValveTime.Data,4);
			Save_Data_Pra();
			iUart.ResetFunc();
			IRAD_Send_Valve_Info();
			if(iUart.WaitFunc(1000) == 0 )
			{
				SuccessFinishFlag = 1;
			}
			break;
		case 0x04: //写入钢瓶生产年月
			memcpy(SysPraData.FlaskTime,iUart.Rxd.FlaskTime.Data,4);
			Save_Data_Pra();
			iUart.ResetFunc();
			IRAD_Send_Valve_Info();
			if(iUart.WaitFunc(1000) == 0 )
			{
				SuccessFinishFlag = 1;
			}
			break;
		case 0x05: //写入阀门编号
			memcpy(SysPraData.ValveId,iUart.Rxd.ValveId.Data,6);
			Save_Data_Pra();
			iUart.ResetFunc();
			IRAD_Send_Valve_Info();
			if(iUart.WaitFunc(1000) == 0 )
			{
				SuccessFinishFlag = 1;
			}
			break;
		case 0x06:  //超级密码开阀
			memcpy(SysPraData.SuperPassword,iUart.Rxd.SuperPassword.Data,12);
			Save_Data_Pra();
			iUart.ResetFunc();
			IRAD_Send_Valve_Info();
			if(iUart.WaitFunc(1000) == 0 )
			{
				SuccessFinishFlag = 1;
			}
			break;
		case 0x07: //写入阀门编号和时间
			memcpy(SysPraData.ValveId, iUart.Rxd.ValveIdTime.ValveId,6);
			memcpy(SysPraData.ValveTime, iUart.Rxd.ValveIdTime.ValveTime,4);
			Save_Data_Pra();
			iUart.ResetFunc();
		  CommResult = 0;
			IRAD_Send_Valve_Info();
			if(iUart.WaitFunc(1000) == 0)
			{
				SuccessFinishFlag = 1;
			}
			break;     
		case 0x08: //写入钢瓶有效使用日期
			if (memcmp(SysPraData.ValveId, iUart.Rxd.FlaskTimeAndPassword.ValveID, 6) != 0)
			{
				CommResult = 3;  //ID不对
			}
			else if (memcmp(SysPraData.Password, iUart.Rxd.FlaskTimeAndPassword.Password, 6) != 0)
			{
				CommResult = 2;  //密码错误
			}
			else
			{
				memcpy(SysPraData.FlaskTime, iUart.Rxd.FlaskTimeAndPassword.FlaskTime, 4);
				Save_Data_Pra();
				CommResult = 0;  //操作成功
			}
			iUart.ResetFunc();
			IRAD_Send_Valve_Info();
			if(iUart.WaitFunc(1000) == 0)
			{
				SuccessFinishFlag = 1;
			}
			break;     	
		default: 
			SuccessFinishFlag = 0;
			CommResult = 0xff;
			break;       
      }
	  if(ValveOpenFlag == 1)
	  {
		ValveOpenFlag = 0;
		LED_ON; 	
		MAGNETIC_ON;
		delay_ms(6000);
		MAGNETIC_OFF;
		LED_OFF; 
		SuccessFinishFlag = 1;
	  }
	}
  }
}

void Save_Data_Pra(void)
{
	uint8_t *p = SysPraData.Password;
	uint8_t i = 0;
	SysPraData.Cs = GetCheckSum(SysPraData.Password, sizeof(SysPraData)-1);
	for(i=0; i < sizeof(SysPraData); i++)
		WriteBuf[i] = *p++;
    flash_write(MyStartAddr,WriteBuf,sizeof(SysPraData));
}

void IRAD_Send_Start(void)
{
    iUart.Txd.R0.Sync = 0x40;
    iUart.Txd.R0.Cmd = 0x0A;
    iUart.Txd.R0.Len = 0x02;
    iUart.Txd.R0.RamdomCode[0] = (uint8_t)((RamdomCode&0xff00)>>8);
    iUart.Txd.R0.RamdomCode[1] = (uint8_t)(RamdomCode&0x00ff);
    iUart.Txd.R0.Cs = GetCheckSum(&iUart.Txd.R0.Sync,sizeof(RESPONSE0)-2);
    iUart.Txd.R0.Tail = 0x23;
    iUart.SendFunc(&iUart.Txd.R0.Sync,sizeof(RESPONSE0));
	  IR_LED_OFF;
}


void IRAD_Send_Valve_Info(void)
{
    iUart.Txd.R1.Sync = 0x40;
    iUart.Txd.R1.Cmd = LastCmd;
    iUart.Txd.R1.Len = 0x09;
    iUart.Txd.R1.RamdomCode[0] = (uint8_t)((RamdomCode&0xff00)>>8);
    iUart.Txd.R1.RamdomCode[1] = (uint8_t)(RamdomCode&0x00ff);
    memcpy(iUart.Txd.R1.ValveId, SysPraData.ValveId, 6);
    iUart.Txd.R1.Result = CommResult; 
    iUart.Txd.R1.Cs = GetCheckSum(&iUart.Txd.R1.Sync,sizeof(RESPONSE1)-2);
    iUart.Txd.R1.Tail = 0x23;
    iUart.SendFunc(&iUart.Txd.R1.Sync,sizeof(RESPONSE1));
	  IR_LED_OFF;
}



uint8_t Irda_SendNByte(uint8_t *data,uint8_t len)
{
    uint8_t i=0;
    USART_Cmd(USART1, DISABLE);
    for(i=0;i<len;i++)
    {
      Irda_SendByte(&data[i]);  
    }
    USART_Cmd(USART1, ENABLE);
    return 0;
}

void Irda_SendByte(uint8_t *data)
{
    uint8_t temp = *data;
    uint8_t i = 0;
    Irda_Modulate(1); //start bit 
    for(i=0;i<8;i++) //data bit
    {   
        if((temp&0x01)==1)
            Irda_Modulate(0);    
        else 
            Irda_Modulate(1);
        temp=temp>>1;
    }
    Irda_Modulate(0);//stop bit
}

void Irda_Modulate(uint8_t state)
{
    if (state) 
	{
		TIM_SetCounter(TIM3, 0);		
		TIM_CCxCmd(TIM3, TIM_Channel_4, TIM_CCx_Enable);
	}
    delay_us(417);
    TIM_CCxCmd(TIM3, TIM_Channel_4, TIM_CCx_Disable); 
}

uint8_t isResetBind(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  /* 使能GPIOA时钟 */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

  /* 配置LED相应引脚PB5*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
  delay_ms(10);
  if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == Bit_RESET)
  {
	  delay_ms(10);
	  if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == Bit_RESET)
	  {
		return 1;
	  }
  }

  return 0;
}


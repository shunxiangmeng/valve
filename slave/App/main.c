#include "stm32f0xx.h"
#include "stdio.h"
#include "string.h"
#include "led.h"
#include "delay.h"
#include "pwm.h"
#include "usart.h"
#include "flash_ee.h"
#include "Include.h"

/* �궨�� --------------------------------------------------------------------*/
#define BUF_SIZE                  128            //BUF����
uint32_t WriteBuf[BUF_SIZE] = {0};
uint32_t ReadBuf[BUF_SIZE];

uint16_t RamdomCode = 0x1234;;
uint8_t LastCmd = 0xff;
uint8_t CommResult = 0xff; //ͨ�Ž��

unsigned char test433[8] = {0x40, 0xA5, 0x02, 0x02, 0x03, 0x04,0x05,0x06};

extern IRDA_COMM iUart;

struct SYS_DATA
{
  uint8_t Password[6];
  uint8_t ValveTime[4];
  uint8_t FlaskTime[4];
  uint8_t ValveId[6];
  uint8_t SuperPassword[12];
  uint8_t Cs;
}SysPraData;


//��������
void FLASH_TEST(void);


uint8_t Irda_SendNByte(uint8_t *data,uint8_t len);
void Save_Data_Pra(void);
void IRAD_Send_Start(void);
void IRAD_Send_Valve_Info(void);

void Irda_SendByte(uint8_t *data);

void Irda_Modulate(uint8_t state);

void Enable_SWD_Pin(void);


/************************************************
�������� �� System_Initializes
��    �� �� ϵͳ��ʼ��
��    �� �� ��
�� �� ֵ �� ��
��    �� �� strongerHuang
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
�������� �� Enable_SWD_Pin
��    �� �� ʹ��SWD����
��    �� �� ��
�� �� ֵ �� ��
��    �� �� strongerHuang
*************************************************/
void Enable_SWD_Pin(void)
{

          /* ʹ��GPIOAʱ�� */
         //RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
         
         GPIO_PinAFConfig(GPIOA,GPIO_Pin_13,GPIO_AF_0);//GPIOA13����Ϊswd
         GPIO_PinAFConfig(GPIOA,GPIO_Pin_14,GPIO_AF_0);//GPIOA14����Ϊswd
         
}

/************************************************
�������� �� SoftwareDelay
��    �� �� �����ʱ
��    �� �� Cnt --- ��ʱ����
�� �� ֵ �� ��
��    �� �� strongerHuang
*************************************************/
void SoftwareDelay(uint32_t Cnt)
{
  while(Cnt--);
}

/************************************************
�������� �� main
��    �� �� ���������
��    �� �� ��
�� �� ֵ �� ��
��    �� �� strongerHuang
*************************************************/
int main(void)
{
  uint8_t SuccessFinishFlag = 0;
  uint8_t i;
  uint8_t *t = NULL;
  uint16_t iFlaskTime = 0;
  uint16_t iTimeTmp = 0;
  uint8_t ValveOpenFlag = 0;
	
 System_Initializes();
	//GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
	//Enable_SWD_Pin();
	
	delay_ms(50);
	
  iUart.WaitFunc = &UART1_Wait;
  iUart.SendFunc = &Irda_SendNByte;
  iUart.ResetFunc = &UART1_Clear;
	
  for(i=0;i<3;i++)
  {
		delay_ms(250);
    LED_ON; 
    delay_ms(250);
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
    memset(SysPraData.Password,0,sizeof(SysPraData));
  }
  
  iFlaskTime = (SysPraData.FlaskTime[0]*10+SysPraData.FlaskTime[1])*12;
  iFlaskTime += SysPraData.FlaskTime[2]*10+SysPraData.FlaskTime[3];
  
  
  USART_Cmd(USART1, DISABLE);
	
	             SysPraData.FlaskTime[0]=0x01;   
               SysPraData.FlaskTime[1]=0x05;   
               SysPraData.FlaskTime[2]=0x00;
               SysPraData.FlaskTime[3]=0x01;
               
               SysPraData.Password [0]=38; 
               SysPraData.Password [1]=32;
               SysPraData.Password [2]=33;
               SysPraData.Password [3]=34;
               SysPraData.Password [4]=39;
               SysPraData.Password [5]=32;
	
  while(1)
  {
    while(SuccessFinishFlag == 1){}
    iUart.ResetFunc();
    IRAD_Send_Start();
    if(iUart.WaitFunc(1000) == 0)
    {
      LastCmd = iUart.Rxd.Buf[1];
      LED_ON;                                      //LED��
      //CommResult = 1;  //��ʱ������
      switch (iUart.Rxd.Buf[1])
      {
		  case 0x01:  //������ʱ��������
			if(memcmp(SysPraData.Password,iUart.Rxd.OpenValve.StationPassword,6) == 0 ||
				memcmp(SysPraData.SuperPassword,iUart.Rxd.SuperPassword.Data,12) == 0)
			{
				iTimeTmp = (iUart.Rxd.OpenValve.CurrentTtim[0]*10+iUart.Rxd.OpenValve.CurrentTtim[1])*12;
                iTimeTmp += iUart.Rxd.OpenValve.CurrentTtim[2]*10+iUart.Rxd.OpenValve.CurrentTtim[3];
                if((iTimeTmp >= iFlaskTime) && ((iTimeTmp-iFlaskTime) < 48))
                {
					CommResult = 0;
                }
			}
			iUart.ResetFunc(); 
			CommResult = 0;//��ʱ������
			IRAD_Send_Valve_Info();
			
			if(iUart.WaitFunc(1000) == 0 )
			{
				ValveOpenFlag = 1;
			}
			break;
		case 0x02: //д��ú��վ����
	
			memcpy(SysPraData.Password,iUart.Rxd.Password.Data,6);
			Save_Data_Pra();
			iUart.ResetFunc(); 
			IRAD_Send_Valve_Info();
			if(iUart.WaitFunc(1000) == 0 )
			{
				SuccessFinishFlag = 1;
			}
			break;
		case 0x03: //д�뷧����������
			memcpy(SysPraData.ValveTime,iUart.Rxd.ValveTime.Data,4);
			Save_Data_Pra();
			iUart.ResetFunc();
			IRAD_Send_Valve_Info();
			if(iUart.WaitFunc(1000) == 0 )
			{
				SuccessFinishFlag = 1;
			}
			break;
		case 0x04: //д���ƿ��������
			memcpy(SysPraData.FlaskTime,iUart.Rxd.FlaskTime.Data,4);
			Save_Data_Pra();
			iUart.ResetFunc();
			IRAD_Send_Valve_Info();
			if(iUart.WaitFunc(1000) == 0 )
			{
				SuccessFinishFlag = 1;
			}
			break;
		case 0x05: //д�뷧�ű��
			memcpy(SysPraData.ValveId,iUart.Rxd.ValveId.Data,6);
			Save_Data_Pra();
			iUart.ResetFunc();
			IRAD_Send_Valve_Info();
			if(iUart.WaitFunc(1000) == 0 )
			{
				SuccessFinishFlag = 1;
			}
			break;
		case 0x06:  //�������뿪��
			memcpy(SysPraData.SuperPassword,iUart.Rxd.SuperPassword.Data,12);
			Save_Data_Pra();
			iUart.ResetFunc();
			IRAD_Send_Valve_Info();
			if(iUart.WaitFunc(1000) == 0 )
			{
				SuccessFinishFlag = 1;
			}
			break;
		case 0x07: //д�뷧�ű�ź�ʱ��
			memcpy(SysPraData.ValveId,iUart.Rxd.ValveIdTime.ValveId,6);
			memcpy(SysPraData.ValveTime,iUart.Rxd.ValveIdTime.ValveTime,4);
			Save_Data_Pra();
			iUart.ResetFunc();
		  CommResult = 0;
			IRAD_Send_Valve_Info();
			if(iUart.WaitFunc(1000) == 0 )
			{
				SuccessFinishFlag = 1;
			}
			break;     
		case 0x08: //д�뿪������͸�ƿ��Чʹ������
			memcpy(SysPraData.Password,iUart.Rxd.FlaskTimeAndPassword.Password,6);
			memcpy(SysPraData.FlaskTime,iUart.Rxd.FlaskTimeAndPassword.FlaskTime,4);
			Save_Data_Pra();
			iUart.ResetFunc();
		  IRAD_Send_Valve_Info();
			if(iUart.WaitFunc(1000) == 0 )
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
    memcpy(iUart.Txd.R1.ValveId,SysPraData.ValveId,6);
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

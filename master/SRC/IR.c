#include "SysTick.h"
#include "ir.h"
#include "string.h"

IR_INFO gIR;




void IR_UartInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure; 

	/* config USART1 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure); 
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* USART1 mode config */
	USART_InitStructure.USART_BaudRate = 2400;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure); 
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);  
	USART_Cmd(USART1, ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
	
	// Enable the USARTy Interrupt 
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;	 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
}


void IR_RxIRQ(void)
{


}

void IR_clear(void)
{
	gIR.rxCount = 0;
	gIR.rxFlag = 0;
	gIR.rxCmd = 0;
	memset((char*)&gIR.rxBuf, 0, sizeof(gIR.rxBuf));
}

unsigned char CheckSum(const u8 *p, const u8 n)
{
	u8 total=0;
	u32 i;
	for(i=0;i<n;i++)
	{
		total += *p++;
	}
	return total;
}


void USART1_IRQHandler(void)
{
	uint32_t len, i;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{	 
		gIR.rxBuf[gIR.rxCount] = USART_ReceiveData(USART1);
		if(gIR.rxBuf[0] != 0x40)  //'@'
		{
			gIR.rxCount = 0;
		}	
		else
		{
			gIR.rxCount++;
			if(gIR.rxCount >= UART1_RX_MAX_LEN)
			{
				gIR.rxCount = 0;
			}
			
			if(gIR.rxCount >= 3 && gIR.rxCount >= gIR.rxBuf[2]+5)
			{
				if(gIR.rxBuf[gIR.rxCount - 2] == CheckSum(gIR.rxBuf, gIR.rxCount-2))
				{
					gIR.rxFlag = 1;
					gIR.rxCount = 0;
					gIR.rxCmd = gIR.rxBuf[1];
					gIR.lastCmd = gIR.rxCmd;
					gIR.randomCode = gIR.rxBuf[3] << 8;
					gIR.randomCode += gIR.rxBuf[4];
					len = gIR.rxBuf[2];
					for (i = 0; i < len; i++)
					{
						gIR.rxData[i] = gIR.rxBuf[3+i];
					}
				}
			}
		}
	} 
}

//PWM ‰≥ˆIR–≈∫≈
static void IR_SendGpioInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* TIM3 clock enable */
	//PCLK1æ≠π˝2±∂∆µ∫Û◊˜Œ™TIM3µƒ ±÷”‘¥µ»”⁄72MHz
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); 

	/*GPIOB clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); 

	/*GPIOB Configuration: TIM3 channel 1 and 2 as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		    
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

static void TIM3_Mode_Config(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	u16 CCR3_Val = 631;

	/* -----------------------------------------------------------------------
	TIM3 Configuration: generate 4 PWM signals with 4 different duty cycles:
	TIM3CLK = 72 MHz, Prescaler = 0x0, TIM3 counter clock = 72 MHz
	TIM3 ARR Register = 999 => TIM3 Frequency = TIM3 counter clock/(ARR + 1)
	TIM3 Frequency = 72 KHz.
	TIM3 Channel3 duty cycle = (TIM3_CCR3/ TIM3_ARR)* 100 = 33%
	----------------------------------------------------------------------- */

	/* Time base configuration */		 
	TIM_TimeBaseStructure.TIM_Period = 1894;      
	TIM_TimeBaseStructure.TIM_Prescaler = 0;	    
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1 ;	
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  

	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	/* PWM1 Mode configuration: Channel1 */
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;	   

	/* PWM1 Mode configuration: Channel3 */
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = CCR3_Val;	

	TIM_OC3Init(TIM3, &TIM_OCInitStructure);	

	TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);

	TIM_ARRPreloadConfig(TIM3, ENABLE);			

	/* TIM3 enable counter */
	TIM_Cmd(TIM3, DISABLE);                 
}


//ÂèëÈÄÅ‰∏Ä‰∏™bitÊï∞ÊçÆ
static void IR_SendBit(uint8_t bit)
{
	if(bit == 0)
	{
		TIM_Cmd(TIM3, ENABLE);   //∑¢ÀÕµƒ ±∫Ú ’µΩµÕµÁ∆Ω
	}
	Delay_us(417);  //2400Ê≥¢ÁâπÁéá(1000*1000/2400)us
	TIM_Cmd(TIM3, DISABLE);
}

//‰∏≤Âè£Êï∞ÊçÆÊ†ºÂºè8-N-1
static void IR_SendByte(uint8_t data)
{
	uint8_t temp = data;
	uint32_t i;
	
	IR_SendBit(0); //Ëµ∑Âßã‰Ωç
	for(i = 0; i < 8; i++)
	{
		if(temp&0x01)
		{
			IR_SendBit(1);
		}
		else
		{
			IR_SendBit(0);
		}
		temp >>= 1;
	}
	IR_SendBit(1); //ÂÅúÊ≠¢‰Ωç
}

void IR_Send(uint8_t *data, uint8_t len)
{
	uint32_t i = 0;
	
	if (data == 0)
	{
		return;
	}
	
	USART_Cmd(USART1, DISABLE);
	for(i = 0; i < len; i++)
	{
		IR_SendByte(data[i]);
	}
	USART_Cmd(USART1, ENABLE);
}

void IR_SendCMD(uint8_t cmd, uint32_t randomCode, uint8_t *dat, uint8_t len)
{
	uint8_t buf[32];
	uint8_t checkSum = 0;
	uint32_t i;
	uint32_t cnt = 0;
	
	buf[cnt++] = '@';
	buf[cnt++] = cmd;
	buf[cnt++] = len+2;
	buf[cnt++] = (randomCode >> 8);
	buf[cnt++] = randomCode & 0xff;
	
	for(i = 0; i < len; i++)
	{
		buf[cnt++] = dat[i];
	}
	
	checkSum = CheckSum(buf, len + 5);
	buf[cnt++] = checkSum;
	buf[cnt++] = 0x23;
	
	IR_clear();
	IR_Send(buf, len + 7);
}

int IR_CmdAndWait(uint8_t cmd, uint32_t randomCode, uint8_t *dat, uint8_t len, uint32_t timeOut)
{
	uint32_t cnt = 0;
	IR_SendCMD(cmd, randomCode, dat, len);
	while(gIR.rxFlag == 0 && cnt < timeOut)
	{
		Delay_ms(1);
		cnt++;
	}
	if (cnt >= timeOut)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

void IR_SendAck(uint32_t randomCode, uint8_t ok)
{
	uint8_t buf[32];
	uint8_t checkSum = 0;
	uint32_t cnt = 0;
	
	buf[cnt++] = '@';
	buf[cnt++] = gIR.lastCmd;
	buf[cnt++] = 3;
	buf[cnt++] = (randomCode >> 8);
	buf[cnt++] = randomCode & 0xff;
	
	buf[cnt++] = ok;
	
	checkSum = CheckSum(buf, 6);
	buf[cnt++] = checkSum;
	buf[cnt++] = 0x23;
	
	IR_clear();
	IR_Send(buf, 8);
}

int IR_GetValveID(uint32_t timeOut)
{
	uint8_t cmd[2] = {0x01};
	uint32_t i = 0;
	int ret = IR_CmdAndWait(0xFE, gIR.randomCode, cmd, 1, timeOut);
	
	for (i = 0; i < 6; i++)
	{
		gIR.valveID[i] = gIR.rxData[i+2];
	}
	
	return ret;
}
//===============================================================================

void IR_Init(void)
{
	IR_UartInit();
	IR_SendGpioInit();
	TIM3_Mode_Config();

}



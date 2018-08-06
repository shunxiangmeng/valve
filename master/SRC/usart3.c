#include "usart3.h"
#include "bluetooth.h"
#include "stdarg.h"	
#include "string.h"	
#include "stdio.h"

extern BLE_INFO gBLE;

extern unsigned char CheckSum(u8 *p, const u8 n);

//uart3接收中断处理函数
void USART3_IRQHandler(void)
{
	u8 res;	      
	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		USART_ClearITPendingBit(USART3,USART_IT_RXNE);		
		res = USART_ReceiveData(USART3);		 
		if (gBLE.rxCount < BLE_COM_RX_CNT_MAX)	
		{			
			gBLE.rxData.buf[gBLE.rxCount++]  = res;	
			
			if (gBLE.isConnect == TRUE)  //蓝牙已连接APP
			{
				if (gBLE.rxData.buf[0] != 0x40 && gBLE.rxData.buf[0] != 'O')
				{
					gBLE.rxCount = 0;
				}
				else
				{
					if (gBLE.rxCount > BLE_COM_RX_CNT_MAX)
					{
						gBLE.rxCount = 0;
					}
					
					if(gBLE.rxCount >= 3 && gBLE.rxCount >= gBLE.rxData.buf[2] + 5 && gBLE.rxData.buf[gBLE.rxCount - 1] == 0x23)
					{
						if(gBLE.rxData.buf[gBLE.rxCount - 2] == CheckSum((unsigned char*)gBLE.rxData.buf, gBLE.rxCount-2))
						{
							gBLE.rxFlag = 1;
							gBLE.rxCmd = gBLE.rxData.buf[1];
							gBLE.rxCount = 0;
						}
						else
						{
							gBLE.rxCount = 0;
							gBLE.rxFlag = 0;
						}
					}
				}
			}
			
		}
	} 	
	
    if (USART_GetFlagStatus(USART3, USART_FLAG_ORE) == SET)
    {
        USART_ClearFlag(USART3, USART_FLAG_ORE);	
        USART_ReceiveData(USART3);				
    }							 
} 

//初始化IO 串口3
//ppclk1:PCLK1时钟频率(Mhz)
//bound:波特率	
void Uart3_Init(u32 bound)
{  
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	                       //GPIOB时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);                          //串口3时钟使能

 	USART_DeInit(USART3);  //复位串口3
	 //USART3_TX   PB10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;                                     //PB10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	                               //复用推挽输出
	
	GPIO_Init(GPIOB, &GPIO_InitStructure);                                         //初始化PB10
 
	//USART3_RX	  PB11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;                          //浮空输入
	GPIO_Init(GPIOB, &GPIO_InitStructure);                                         //初始化PB11
	
	USART_InitStructure.USART_BaudRate = bound;                                    //波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;                    //字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;                         //一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;                            //无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	               //收发模式
  
	USART_Init(USART3, &USART_InitStructure);       //初始化串口3
  

	USART_Cmd(USART3, ENABLE);                      //使能串口 
	
	//使能接收中断
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);  //开启中断 
	USART_ITConfig(USART3, USART_IT_ERR, ENABLE);	
	USART_ITConfig(USART3, USART_IT_TXE, DISABLE); 
	
	//设置中断优先级
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0 ;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;		//子优先级0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
}

//串口3,printf 函数
//确保一次发送数据不超过USART3_MAX_SEND_LEN字节
void Uart3_sendStr(char* fmt, ...)  
{  
	uint32_t i,j; 
	va_list ap; 
	va_start(ap, fmt);
	memset(gBLE.txBuf, 0, sizeof(gBLE.txBuf));	
	vsprintf((char*)gBLE.txBuf, fmt, ap);
	va_end(ap);
	i = strlen((const char*)gBLE.txBuf);		//此次发送数据的长度
	for(j = 0; j < i; j++)							//循环发送数据
	{
		while(USART_GetFlagStatus(USART3,USART_FLAG_TC) == RESET); //循环发送,直到发送完毕   
		USART_SendData(USART3, gBLE.txBuf[j]); 
	} 
}


void Uart3_sendData(char* data, uint8_t len)
{
	uint32_t i = 0;
	for (i = 0; i < len; i++)
	{
		while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET); //循环发送,直到发送完毕  
		USART_SendData(USART3, data[i]); 		
	}
}


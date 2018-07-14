/**
  **********************************  STM32F030F4  ***********************************
  * @�ļ���     �� usart1.c
  * @����       �� liucf
  * @��汾     �� V2.2.0
  * @�ļ��汾   �� V1.0.0
  * @����       �� 2017��04��10��
  * @ժҪ       �� 
  ******************************************************************************/

#include "usart.h"
#include "stdio.h"
#include "string.h"
#include "delay.h"
#include "stm32f0xx_gpio.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_usart.h"
#include "stm32f0xx_usart.h"
#include "Include.h"

IRDA_COMM iUart;

/* USART��ʼ�� */
void USART1_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStruct;
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);  //ʹ��GPIOA��ʱ��
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);//ʹ��USART��ʱ��
	/* USART1�Ķ˿����� */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_1);//����PA2�ɵڶ���������	TX
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_1);//����PA3�ɵڶ���������  RX	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	

	/* USART1�Ļ������� */
	USART_InitStructure.USART_BaudRate = 2400;              //������
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART1, &USART_InitStructure);		
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);           //ʹ�ܽ����ж�
	USART_Cmd(USART1, ENABLE);                             //ʹ��USART1
	
	/* USART1��NVIC�ж����� */
	NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPriority = 0x02;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
				
}

//=============================================================================
//�ļ����ƣ�
//���ܸ�Ҫ��USART1�жϺ���
//����˵������
//�������أ���
//=============================================================================
void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
	
		iUart.Rxd.Buf[iUart.RxCnt]=USART_ReceiveData(USART1);
		if(iUart.RxCnt == 0 && iUart.Rxd.Buf[iUart.RxCnt] != '@')
		{
			iUart.RxCnt = 0;
		}
		else 
		{
			iUart.RxCnt++;
			if(iUart.RxCnt >= iUart.Rxd.Buf[2]+5)
			{
				if (iUart.Rxd.Buf[iUart.RxCnt-2] == GetCheckSum(&iUart.Rxd.Buf[0],iUart.RxCnt-2)
				&&iUart.Rxd.Buf[iUart.RxCnt-1] == 0x23)
				{
					iUart.RxFlag = 1;
				}
				else
				{
					iUart.RxFlag = 0; 
					iUart.RxCnt=0;
				}
			}        
		}
		if(iUart.RxCnt>=MAXCNTRX1)
			iUart.RxCnt=0;//���ڽ��� MAXCNTRX2=64�ֽ�
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}		
}

/************************************************
�������� �� UART1_SendByte
��    �� �� UART1����һ���ַ�
��    �� �� Data --- ����
�� �� ֵ �� ��
��    �� �� strongerHuang
*************************************************/
void UART1_SendByte(uint8_t Data)
{
	USART_SendData(USART1,Data);
	while (USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
}

/************************************************
�������� �� UART1_SendNByte
��    �� �� ����1����N���ַ�
��    �� �� pData ---- �ַ���
            Length --- ����
�� �� ֵ �� ��
��    �� �� strongerHuang
*************************************************/
uint8_t UART1_SendNByte(uint8_t *pData, uint8_t Length)
{
  while(Length--)
  {
    UART1_SendByte(*pData);
    pData++;
  }
  return 0;
}

/************************************************
�������� �� UART1_Printf
��    �� �� ����1��ӡ���
��    �� �� String --- �ַ���
�� �� ֵ �� ��
��    �� �� strongerHuang
*************************************************/
void UART1_Printf(uint8_t *String)
{
  while((*String) != '\0')
  {
    UART1_SendByte(*String);
    String++;
  }
}

int8_t UART1_Wait(uint16_t DelayCnt) 
{
  uint16_t i = DelayCnt;
  while (i--) 
  {
    delay_ms(1);
    if(iUart.RxFlag == 0x01)
    return 0;
  }
  return -1;
}

void UART1_Clear(void) 
{
    iUart.RxCnt = 0;
    iUart.RxFlag = 0;
    iUart.TxFlag = 0;
    memset(iUart.Rxd.Buf,0,MAXCNTRX1);
}

unsigned char GetCheckSum(const uint8_t *p, uint8_t n) /* how many chars to process */
{
    uint8_t total=0;
    uint8_t i;
    for(i=0;i<n;i++)
    {
      total += *p++;
    }
  return total;
}


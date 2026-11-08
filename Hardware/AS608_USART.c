#include "stm32f10x.h"                  // Device header
#include "AS608_USART.h"

#define USART2_Tx GPIO_Pin_2
#define USART2_Rx GPIO_Pin_3

uint8_t USART2_RX_BUF[USART2_MAX_RECV_LEN]; 
uint8_t USART2_TX_BUF[USART2_MAX_SEND_LEN]; 
uint16_t USART2_RX_STA = 0;

static uint8_t rx_step=0;
static uint8_t rx_need;
static uint16_t rx_len;

void AS608_USART_GPIO_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitTypeDef	GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 57600;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_2;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART2,&USART_InitStructure);
	USART_Cmd(USART2,ENABLE);
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
}

void USART_Reset(void)
{
//	uint32_t primask = __get_PRIMASK(); //判断当前是否有中断开启，0：有；非0：没有
//	__disable_irq();//关闭所有可屏蔽中断，避免其他中断修改当前全局变量造成冲突
	
	USART2_RX_STA=0;
	rx_need=0;
	rx_len=0;
	rx_step=0;
	
	/*		防止未清除的标志位使程序卡死在串口中断服务函数		*/
	(void)USART2->SR;//读取SR后，硬件自动清除中断标志位
	(void)USART2->DR;//读取DR后，硬件自动清除中断标志位
	
	//	if(!primask)		//如果最开始有中断开启，则开启，否则不开；
//		__enable_irq();
}

void USART2_IRQHandler(void)
{
	uint8_t ReceiveData;
	
	if(USART_GetITStatus(USART2,USART_IT_RXNE)==SET)//收到数据
	{
		ReceiveData=USART_ReceiveData(USART2);
		
		if(USART2_RX_STA & 0x8000)		
			return;

		switch(rx_step)
		{
			case 0:
				if(ReceiveData==0xEF)
				{
					USART2_RX_BUF[0]=ReceiveData;
					rx_step=1;
					USART2_RX_STA=1;
				}
				break;
			case 1:
				if(ReceiveData==0x01)
				{
					USART2_RX_BUF[1]=ReceiveData;
					rx_step=2;
					rx_need=7;
					USART2_RX_STA=2;
				}
				else if(ReceiveData!=0xEF)
				{
					USART2_RX_STA=0;
					rx_need=0;
					rx_len=0;
					rx_step=0;
				}
				break;
			case 2:
				USART2_RX_BUF[USART2_RX_STA++]=ReceiveData;
				if(--rx_need==0)
				{
					rx_len= (USART2_RX_BUF[7]<<8) | USART2_RX_BUF[8];
					if(rx_len<3 || (rx_len+9) >USART2_MAX_RECV_LEN)
					{
						USART2_RX_STA=0;
						rx_need=0;
						rx_len=0;
						rx_step=0;
						break;
					}
					else
					{
						rx_need=rx_len;
						rx_step=3;
					}
					
				}
				
				break;
			case 3:
				USART2_RX_BUF[USART2_RX_STA++]=ReceiveData;
				if(--rx_need==0)
				{
					USART2_RX_STA |= 0x8000;
					rx_step=0;
				}
				break;
		}
	}
}

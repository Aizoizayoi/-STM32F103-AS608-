#include "stm32f10x.h"                  // Device header
#include "AS608_USART.h"

/** @defgroup USART_PIN_DEFINE
  * 可改PA9和PA10，USART1
  * @{
  */
#define USART2_Tx GPIO_Pin_2
#define USART2_Rx GPIO_Pin_3

//接收和发送数据组
uint8_t USART2_RX_BUF[USART2_MAX_RECV_LEN]; 	//接收缓冲,最大USART2_MAX_RECV_LEN字节
uint8_t USART2_TX_BUF[USART2_MAX_SEND_LEN]; 	//发送缓冲,最大USART2_MAX_SEND_LEN字节
uint16_t USART2_RX_STA = 0;						//接收状态标识符与记录接收数，最高位用于标识接收状态，最高位=0：未接收完；最高位=1：接收完成
												//其余位用于记录接收到第几个字节


static uint8_t rx_step=0;						//状态机状态位
static uint8_t rx_need;							//需要连续接收的字节数
static uint16_t rx_len;							//用于记录包长度

void AS608_USART_GPIO_Init(void)
{
	/*开启USART2和GPIOA时钟树*/
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	/*配置GPIO*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;		//硬件USART模式，使用复用推挽
	GPIO_InitStructure.GPIO_Pin = USART2_Tx;			//发送引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;		//上拉输入，USART闲置时默认高电平，低电平标志开始，所以用上拉
	GPIO_InitStructure.GPIO_Pin = USART2_Rx;			//接收引脚
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	/*配置硬件USART*/
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 57600;			//AS608波特率57600
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;		//无硬件流
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;		//同时开启接收和发送
	USART_InitStructure.USART_Parity = USART_Parity_No;					//无校验位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;				//停止符1位。AI说AS608常见1位，我看手册是2位.但是实验下来用2位长时间运行后确实会造成接收错误，所以就1位吧
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;			//接收8位
	USART_Init(USART2,&USART_InitStructure);							//初始化USART2
	USART_Cmd(USART2,ENABLE);											//使能USART2
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);						//开启接受寄存器非空硬件中断
	
	/*配置中断优先级*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);						//组2
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;					//中断通道为USART2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;						//开启中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;			//中断优先级1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;					//次优先级1。注意：串口的USART1是优先级2，所以串口不会打断AS608的接收
	NVIC_Init(&NVIC_InitStructure);										//初始化中断优先级
	
}


/**
* @brief  用于在VertifyAck函数中接收的应答包被判定为非法时，重置接收标识符和清除寄存器标志位，重新接收
  * @param  none
  * @retval none
  */


void USART_Reset(void)
{
	uint32_t primask = __get_PRIMASK(); 		//判断当前是否有中断开启，0：有；非0：没有
	__disable_irq();							//关闭所有可屏蔽中断，即USART2_IRQHandler，避免同时修改当前全局变量造成冲突
	
	USART2_RX_STA=0;
	rx_need=0;
	rx_len=0;
	rx_step=0;
	
	/*		防止未清除的标志位使程序卡死在串口中断服务函数		*/
	(void)USART2->SR;							//读取SR后，硬件自动清除中断标志位
	(void)USART2->DR;							//读取DR后，硬件自动清除中断标志位
	
	if(!primask)							//如果最开始有中断开启，则开启，否则不开；
	__enable_irq();
}

//接收格式
//----2byte 	4byte 	1byte 		2byte		1byte	Nbyte		2byte
//----包头		地址	包标识		包长度		确认码	返回参数	校验和
//包长=确认码+返回参数+校验和，最小长度为3，即无返回参数的情况，最大长度受限于接收缓冲数组的长度设定，此项目为USART2_MAX_RECV_LEN=400

/**
  * @brief  AS608应答包接收的中断函数。使用状态机接收应答包
  * @param  none
  * @retval none
  */

void USART2_IRQHandler(void)
{
	uint8_t ReceiveData;
	
	if(USART_GetITStatus(USART2,USART_IT_RXNE)==SET)		//是否收到数据
	{
		ReceiveData=USART_ReceiveData(USART2);
		
		if(USART2_RX_STA & 0x8000)							//接收到的数据未被取走
			return;

		switch(rx_step)
		{
			case 0:											//状态0：接收包头0xEF
				if(ReceiveData==0xEF)						//接收到0xEF
				{
					USART2_RX_BUF[0]=ReceiveData;			//存入接收缓冲数组第0位
					rx_step=1;								//进入状态1
					USART2_RX_STA=1;						//接收到第一个字节
				}
				break;
			case 1:											//状态1：接收包头0x01
				if(ReceiveData==0x01)
				{
					USART2_RX_BUF[1]=ReceiveData;			//存入接收缓冲数组第1位			
					rx_step=2;								//进入状态2
					rx_need=7;								//后续需要连续接收7个字符
					USART2_RX_STA=2;
				}
				else if(ReceiveData!=0xEF)					//若非是0x01，判断是否是新的包头0xEF
				{
					USART2_RX_STA=0;						//未接收完字节
					rx_need=0;								//不需要连续接收字符，重置需要接收数
					rx_len=0;								
					rx_step=0;								//返回状态0
				}
				break;
			case 2:											//状态2，连续接收7位字节，包括地址、包标识、包长
				USART2_RX_BUF[USART2_RX_STA++]=ReceiveData;	//依次存入接收缓冲数组，USART2_RX_STA接收数增加
				if(--rx_need==0)							//每次新的字节到就会触发一次中断，在need=0前，都是状态2，每次都会进case2触发--rx_need
				{
					rx_len= (USART2_RX_BUF[7]<<8) | USART2_RX_BUF[8];			//7个字节接收完，将包长存入rx_need
					if(rx_len<3 || (rx_len+9) >USART2_MAX_RECV_LEN)				//如果包长小于3或超出最大接收长度，则为非法接收，重置各标识符并返回状态0重新接收
					{
						USART2_RX_STA=0;					//置未接受完
						rx_need=0;							//
						rx_len=0;							//
						rx_step=0;							//返回状态0
						break;
					}
					else									//否则为正常接收
					{
						rx_need=rx_len;						//后续需要接收的字节数为包长即rx_len
						rx_step=3;							//进入状态3
					}
					
				}
				
				break;
			case 3:
				USART2_RX_BUF[USART2_RX_STA++]=ReceiveData;	//连续接收
				if(--rx_need==0)							//直到接收完毕
				{
					USART2_RX_STA |= 0x8000;				//置USART2_RX_STA最高位为1，标识应答包接收完毕
					rx_step=0;								//返回状态0，接收下一新应答
				}
				break;
		}
	}
}

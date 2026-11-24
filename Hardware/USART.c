#include "USART.h"
#include "Delay.h"

/** @defgroup USART_PIN_DEFINE
  * 可改PA2和PA3，USART2
  * @{
  */

#define USART1_TX GPIO_Pin_9
#define USART1_RX GPIO_Pin_10

/**
  * @brief  USART1的GPIO引脚初始化，使用PA9作为TX，PA10作为RX
  * @param  None
  * @retval None
  */
void USART_GPIO_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);		//使能USART1时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);		//使能GPIOA时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);			//使能TIM2中断，用于超时退出
	
	/*配置GPIO*/
	GPIO_InitTypeDef GPIO_InitStructure;						//定义GPIO初始化结构体
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;				//USART硬件控制发送，使用复用推挽
	GPIO_InitStructure.GPIO_Pin = USART1_TX;					//发送数据线，pin9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;				//接收数据线，USART默认高电平，起始位为低电平，所以选上拉输入
	GPIO_InitStructure.GPIO_Pin = USART1_RX;					//接收数据线，pin10
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	/*配置USART*/
	USART_InitTypeDef USART_InitStructure;						//定义USART初始化结构体
	USART_InitStructure.USART_BaudRate = 9600;					//波特率9600
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;		//不使用硬件流控；(可防数据丢失，高速时使用)
	USART_InitStructure.USART_Mode = USART_Mode_Rx |USART_Mode_Tx;						//使能接收和发送
	USART_InitStructure.USART_Parity = USART_Parity_No;			//不使用奇偶校验位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;		//使用1位停止位
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;	//使用8位数据位
	USART_Init(USART1,&USART_InitStructure);					//初始化USART1
	
	USART_Cmd(USART1,ENABLE);									//使能USART1
	
	/*配置定时器TIM2*/
	TIM_TimeBaseInitTypeDef TimeBaseInitStructure;				//定义TIM时基单元结构体
	TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;		//无外部捕获，时钟分频因子不分频
	TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Down;	//向下计数模式
	TimeBaseInitStructure.TIM_Period = 10000 - 1;				//自动重装载值10000
	TimeBaseInitStructure.TIM_Prescaler = 7200 - 1;				//预分频7200
	TimeBaseInitStructure.TIM_RepetitionCounter = 0;			//重复计数器，通用定时器无此配置
	TIM_TimeBaseInit(TIM2,&TimeBaseInitStructure);
	
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);					//使能计时器更新中断
	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);					// TimeBaseInit 会置位更新标志，先清掉
	TIM_Cmd(TIM2,DISABLE);										//失能计时器，需要时再开启
	
	/*配置中断优先级*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);				
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;			//比AS608的接收中断优先级低1
	NVIC_Init(&NVIC_InitStructure);
}




/**
  * @brief  等待置位
  * @param  None
  * @retval None
  */
uint8_t WaitBusy(USART_TypeDef* USARTx, uint16_t USART_FLAG)
{
	int32_t TimeOutFlag=100000;			//ms级
	
	while(USART_GetFlagStatus(USARTx,USART_FLAG)==RESET)
	{
		TimeOutFlag--;
		if(TimeOutFlag==0)
		{
			return 0;
		}
	}
	return 1;
}

/**
  * @brief  发送数据
  * @param  Byte：需要发送的数据
  * @retval None
  */
void USART_SendByte(uint8_t Byte)
{
	USART_SendData(USART1,Byte);
	(void)WaitBusy(USART1,USART_FLAG_TXE);	//等待发送寄存器空
}


/**
  * @brief  接收数据
  * @param  None
  * @retval 接收到的1Byte数据
  */
uint8_t USART_ReceiveByte(void)
{
	if(!WaitBusy(USART1,USART_FLAG_RXNE))	//等待接收寄存器非空
		(void)USART_ReceiveData(USART1);
	return USART_ReceiveData(USART1);
}

/**
  * @brief  接收数据
* @param  ReceiveByte:将接收到的数据传回
  * @retval 接收标识符：1=接收到了；0=超时未接收到数据
  */
uint8_t USART_ReceiveByteFlag(uint8_t *ReceiveByte)
{
	int32_t TimeOutFlag=100;
	while(USART_GetFlagStatus(USART1,USART_FLAG_RXNE)==RESET)
	{
		Delay_ms(1);
		TimeOutFlag--;
		if(TimeOutFlag==0)
		{
			return 0;
		}
	}
	*ReceiveByte=(uint8_t)USART_ReceiveData(USART1);
	return 1;
}


/**
  * @brief  发送字符串
  * @param  String：需要发送的字符串,字符串的每个字符均以8Byte发送，字符串以'\0'结尾
  * @retval None
  */
void USART_SendString(char *String)
{
	uint8_t i=0;
	for(i=0;String[i]!='\0';i++)//通过\0判断字符串结束
	{
		USART_SendByte(String[i]);//发送字符串的每个字符
	}
	
}

/**
  * @brief  判断字符是否为允许的字符，允许的字符为ASCII码表中0x21~0x7E之间的字符，即字母、数字和常用符号
  * @param  ch：需要判断的字符
  * @retval 1 or 0，1表示允许，0表示不允许
  */

uint8_t USART_IsAllowedChar(char ch)
{
	if(ch >= 0x21 && ch <= 0x7E)
		return 1;			//发送允许标志位
	return 0;				//发送不允许标志位
}


/**
  * @brief  USART接收字符，并返回状态标识符和传回字符串
  * @param  ReceiveString
  * @retval 1表示接收成功，0表示数据非法或溢出，2表示约30秒无输入
  * @note 	1)溢出、超长、超时、非法返回对应标识符。
  * 		2)设有定时器，30s无数据，退出接收，返回标识符。
  * 		3)串口输入没有以回车结尾依然可以接收，通过传输间隔判断连续的传输是否结束
*/

volatile uint8_t TimeOutFlag=0;
volatile uint16_t CounterDown=0;

uint8_t USART_ReceiveString(char *ReceiveString)
{
	char ch;
	uint8_t OverflowFlag=0,illegalFlag=0,ifdoneFlag=0,ORE_Flag=0;		//溢出标识符、字符非法标识符、接收完成标识符
	uint16_t idle_us;										//累加延时

	/*重置计数器*/
	TIM_Cmd(TIM2,DISABLE);									//失能TIM2配置参数
	TIM_SetCounter(TIM2,10000-1);							//使CNT重新回到10000向下计数
	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);				//清除更新标志位
	CounterDown=0;											//重置计时
	TimeOutFlag=0;											//重置超时标识符

	TIM_Cmd(TIM2,ENABLE);									//计时开始
	
	/*接收字符*/
	ReceiveString[0] = '\0';							//初始化字符串为空，避免了指针指向未初始化或内容随机的野内存。
	while(1)											//通过循环不断接收串口发送的数据进行判断
	{
		if(USART_GetFlagStatus(USART1,USART_FLAG_ORE)==SET)		//判断是否发生溢出错误
		{
			(void)USART1->SR;
			(void)USART1->DR;							//若溢出则接收数据。先读取SR寄存器，再读取DR寄存器后，硬件自动清除溢出标志位
			ORE_Flag=1;
			break;
		}
		
		if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET)//判断接收缓冲区是否有非空，非空则有数据输入，进行读取并判断
		{
			idle_us = 0;								//延迟计数初始化
			ch = (char)USART_ReceiveByte();				//将接收的数据转换成char并存入ch进行后续判断(char和uint8一样，只是为了规范和易懂性选择使用char)
			if(ch == '\n' || ch == '\r')				//\r回车，\n换行，均表示接输入结束
			{
				if(ifdoneFlag == 0)						//若ifdoneFlag为0，则表示没有接收到任何数据，继续等待输入
					continue;
				break;									//否则为输入字符串结束，跳出循环，进行后续判断
			}
			if(!USART_IsAllowedChar(ch))				//判断接收到的字符是否为允许的字符，若不允许则丢弃，继续接收
			{
				illegalFlag = 1;						//若接收到非法字符则标记为非法，不做输出，继续接收
				continue;
			}
			
			if(ifdoneFlag < USART_STR_MAX_LEN - 1)		//如果数据既不是结束标识符，也合法，则存入ReceiveString数组中，ifdoneFlag计数器加一，且判断数组长度是否超过最大长度，若超过则标记溢出
				ReceiveString[ifdoneFlag++] = ch;
			else
				OverflowFlag = 1;						//若接收到的字符超过了最大长度，则标记溢出，继续接收，直到接收到回车为止
		}
		else if(TimeOutFlag==1)
		{
			break;
		}
		else if(ifdoneFlag > 0)							//此为串口助手没有打回车就点击发送的状况，有输入但没有接收到结束标识符，则在一定等待后自动跳出循环，并接收没有回车的数据
		{
			Delay_us(50);
			idle_us += 50;
			if(idle_us > 3000)							//3ms，串口接收间隔通常为us级，3ms未有输入说明输入结束。*PS：如果是hal库可以用系统滴答时钟SysTick定时，就不用担心阻塞了。
				break;
		}
		
	}
	
	/* 重置定时器参数*/
	TIM_Cmd(TIM2,DISABLE);
	TIM_SetCounter(TIM2,10000-1);	
	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
	CounterDown=0;
	

	ReceiveString[ifdoneFlag]='\0';						//在接收到回车后，给字符串添加结束标志符'\0'，以便后续使用字符串函数进行处理
	if(TimeOutFlag==1)									//等待超时，未操作30s，返回标识符2
	{
		TimeOutFlag=0;
		return 2;
	}
	TimeOutFlag=0;
	if(illegalFlag==1 || ifdoneFlag==0 || OverflowFlag==1 || ORE_Flag==1)//如果数据非法，则直接抛弃数组后续的字符，并置标志位
	{
		ReceiveString[0]='\0';//将字符串置为空，避免了指针指向未初始化或内容随机的野内存。
		return 0;
	}
	
	return 1;//返回接收成功标志位
}

/**
  * @brief  未输入计数，每秒1中断，中断30次后共30s，设置超时标识符
  * @param  none
  * @retval none

*/

void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2,TIM_IT_Update)!= RESET)		//触发更新，即10000倒计数结束
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);		//清除更新标志位
		CounterDown++;
		if(CounterDown>=30)								//30s未操作
		{
			CounterDown=0;								//计时重置
			TIM_Cmd(TIM2,DISABLE);						//关闭计数器
			TimeOutFlag=1;								//超时标识符置位
		}
	}
}

/**
  * @brief  printf重定向到USART1，可通过printf函数直接输出到串口进行显示
  * @param  ch：需要发送的字符，*f：printf函数的文件指针，通常为stdout
  * @retval ch：接收到的字符

*/
int fputc(int ch, FILE *f)
{
	USART_SendByte(ch);
	return ch;
}



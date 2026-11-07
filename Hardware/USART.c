#include "USART.h"
#include "Delay.h"

/** @defgroup USART_PIN_DEFINE
  * @{
  */

#define USART1_TX GPIO_Pin_9
#define USART1_RX GPIO_Pin_10

/** @defgroup W25Q64_STRING_LEN_DEFINE
  * @{
  */
#define USART_STR_MAX_LEN 16

/**
  * @brief  USART1的GPIO引脚初始化，使用PA9作为TX，PA10作为RX
  * @param  None
  * @retval None
  */
void USART_GPIO_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);//使能USART1时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//使能GPIOA时钟
	
	/*配置GPIO*/
	GPIO_InitTypeDef GPIO_InitStructure;//定义GPIO初始化结构体
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//USART硬件控制发送，使用复用推挽
	GPIO_InitStructure.GPIO_Pin = USART1_TX;//发送数据线，pin9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;//接收数据线，上拉输入
	GPIO_InitStructure.GPIO_Pin = USART1_RX;//接收数据线，pin10
	GPIO_Init(GPIOA,&GPIO_InitStructure);

	/*配置USART*/
	USART_InitTypeDef USART_InitStructure;//定义USART初始化结构体
	USART_InitStructure.USART_BaudRate = 9600;//波特率9600
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//不使用硬件流控；(可防数据丢失，高速时使用)
	USART_InitStructure.USART_Mode = USART_Mode_Rx |USART_Mode_Tx;//使能接收和发送
	USART_InitStructure.USART_Parity = USART_Parity_No;//不使用奇偶校验位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//使用1位停止位
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//使用8位数据位
	USART_Init(USART1,&USART_InitStructure);//初始化USART1
	
	USART_Cmd(USART1,ENABLE);//使能USART1
}

/**
  * @brief  USART1的GPIO引脚初始化，使用PA9作为TX，PA10作为RX
  * @param  None
  * @retval None
  */
void WaitBusy(USART_TypeDef* USARTx, uint16_t USART_FLAG)
{
	int32_t TimeOut=100000;
	
	while(USART_GetFlagStatus(USARTx,USART_FLAG)==RESET)
	{
		TimeOut--;
		if(TimeOut==0)
		{
			break;
		}
	}
}

/**
  * @brief  发送数据
  * @param  Byte：需要发送的数据
  * @retval None
  */
void USART_SendByte(uint8_t Byte)
{
	USART_SendData(USART1,Byte);
	WaitBusy(USART1,USART_FLAG_TXE);
}


/**
  * @brief  接收数据
  * @param  None
  * @retval 接收到的1Byte数据
  */
uint8_t USART_ReceiveByte(void)
{
	WaitBusy(USART1,USART_FLAG_RXNE);
	return USART_ReceiveData(USART1);
}

/**
  * @brief  接收数据
* @param  ReceiveByte:将接收到的数据传回
  * @retval 接收标识符：1=接收到了；0=超时未接收到数据
  */
uint8_t USART_ReceiveByteFlag(uint8_t *ReceiveByte)
{
	int32_t TimeOut=100;
	while(USART_GetFlagStatus(USART1,USART_FLAG_RXNE)==RESET)
	{
		Delay_ms(1);
		TimeOut--;
		if(TimeOut==0)
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
		return 1;//发送允许标志位
	return 0;//发送不允许标志位
}


/**
  * @brief  USART接收字符串，串口输入回车则表示输入结束
  * @param  ReceiveString
  * @retval 0或1，0表示接收数据失败或非法，1表示接收成功
  * @note 接收的字符需要进行合法判断，要属于可输入的字符范围，若接收到非法字符则丢弃，继续接收，直到接收到回车为止
*/


uint8_t USART_ReceiveString(char *ReceiveString)
{
	char ch;
	uint8_t Overflow=0,illegal=0,ifdone=0;
	uint16_t idle_us;	//累加延时

	ReceiveString[0] = '\0';//初始化字符串为空，避免了指针指向未初始化或内容随机的野内存。
	while(1)//通过循环不断接收串口发送的数据进行判断
	{
		if(USART_GetFlagStatus(USART1,USART_FLAG_ORE)==SET)		//判断是否发生溢出错误
		{
			(void)USART_ReceiveByte();//若溢出则接收数据。先读取SR寄存器，再读取DR寄存器后，硬件自动清除溢出标志位
		}
		
		if(USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == SET)//判断接收缓冲区是否有非空，非空则有数据输入，进行读取并判断
		{
			idle_us = 0;//延迟计数初始化
			ch = (char)USART_ReceiveByte();//将接收的数据转换成char并存入ch进行后续判断(char和uint8一样，只是为了规范和易懂性选择使用char)
			if(ch == '\n' || ch == '\r')//\r回车，\n换行，均表示接输入结束
			{
				if(ifdone == 0)//若ifdone为0，则表示没有接收到任何数据，继续等待输入
					continue;
				break;//否则为输入字符串结束，跳出循环，进行后续判断
			}
			if(!USART_IsAllowedChar(ch))//判断接收到的字符是否为允许的字符，若不允许则丢弃，继续接收
			{
				illegal = 1;//若接收到非法字符则标记为非法，不做输出，继续接收
				continue;
			}
			if(ifdone < USART_STR_MAX_LEN - 1)//如果数据既不是结束标识符，也合法，则存入ReceiveString数组中，ifdone计数器加一，且判断数组长度是否超过最大长度，若超过则标记溢出
				ReceiveString[ifdone++] = ch;
			else
				Overflow = 1;//若接收到的字符超过了最大长度，则标记溢出，继续接收，直到接收到回车为止
		}
		else if(ifdone > 0)//此为串口助手没有打回车就点击发送的状况，有输入但没有接收到结束标识符，则在一定等待后自动跳出循环，并接收没有回车的数据
		{
			Delay_us(50);
			idle_us += 50;
			if(idle_us > 3000)
				break;
		}
	}
	ReceiveString[ifdone]='\0';//在接收到回车后，给字符串添加结束标志符'\0'，以便后续使用字符串函数进行处理
		
	if(illegal==1 || ifdone==0 || Overflow==1)//如果数据非法，则直接抛弃数组后续的字符，并置标志位
	{
		ReceiveString[0]='\0';//将字符串置为空，避免了指针指向未初始化或内容随机的野内存。
		return 0;
	}
	return 1;//返回接收成功标志位
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


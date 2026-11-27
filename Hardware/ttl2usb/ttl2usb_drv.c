#include "ttl2usb/ttl2usb_drv.h"
#include "usart.h"
#include "usart_callback.h"

#include <stdio.h>

HAL_StatusTypeDef start_status;
CH340_USART_StatusTypeDef rx_status;
volatile uint8_t rx_ifdone=0;				//0:没有接收完成； 1：接收完成
volatile uint16_t rx_len=0;
uint32_t start_tick;            


static uint8_t USART_IsAllowedChar(const uint8_t *String, uint16_t len)
{
	uint16_t i;                                                                                // 当前检查的字节下标

	for (i = 0; i < len; i++)                                                                  // 只检查收到的字节，不检查结束符
	{
		if (String[i] < 0x20U || String[i] > 0x7EU)                                            // 小于空格或大于 '~' 都不可打印
			return 1U;                                                                         // 发现非法字符，停止检查
	}
	return 0U;                                                                                 // 这一段字节全部可打印
}

CH340_USART_StatusTypeDef CH340_ReceiveString(uint8_t *ReceiveString,uint16_t ReceiveSize)
{
	rx_status=USART_OK;
	if(huart1.RxState==HAL_UART_STATE_READY && 					//空闲状态且有溢出标识符时，清除溢出
		(__HAL_UART_GET_FLAG(&huart1,UART_FLAG_ORE)==SET))
	{
		__HAL_UART_CLEAR_OREFLAG(&huart1);  
	}
	
	start_status = HAL_UARTEx_ReceiveToIdle_IT(&huart1, ReceiveString, ReceiveSize-1);
	start_tick=HAL_GetTick();
	
	while(rx_ifdone==0)		//等待接收
	{
		if(HAL_GetTick()-start_tick>CH340_RX_TIMEOUT_MS && huart1.RxState==HAL_UART_STATE_BUSY_RX)
		{
			
			(void)HAL_UART_AbortReceive(&huart1); 
			printf("Waiting Rx Receive Timeout. Receive Exit.\r\n"); 
			rx_status=USART_TIMEOUT;
			break;
		}
		else if(rx_status==USART_ERROR)
		{
			printf("Waiting Rx Receive Error. Receive Exit.\r\n"); 
			break;
		}
	}
	
	if(rx_ifdone==1)
	{
		for(uint16_t i=0;i<rx_len;i++)
		{
			if(ReceiveString[i]=='\r' || ReceiveString[i]=='\n')
			{
				rx_len=i;
				break;
			}

		}
		if(rx_len == 0 || ReceiveString[0] == '\r' || ReceiveString[0] == '\n')
		{
			rx_status=USART_ERROR;
		}
		else if(rx_len>CH340_RX_Rec_SIZE-1 || rx_len>ReceiveSize-1)
		{
			rx_status=USART_Overflow;
		}
		
		else if(USART_IsAllowedChar(ReceiveString,rx_len)!=0)
		{
			rx_status=USART_illegal;
		}
		
		else{
			rx_status=USART_OK;
			ReceiveString[rx_len]='\0';
		}
	}
	start_tick=0;
	rx_ifdone=0;
	return rx_status;                                                                           // 返回接收状态
}

void USART1_RxEventCallback(uint16_t Size)
{
	rx_len=Size;
	rx_ifdone=1;	
}                                                                 

void USART1_ErrorCallback()
{
	rx_len=0;
	rx_ifdone=1;
	rx_status=USART_ERROR;
}

/**
* @brief  把 printf 的输出送到 USART1
* @param  ch 要发送的字符
* @param  f  标准库要求的文件指针，这里不使用
* @retval 已发送的字符
*/
int fputc(int ch, FILE *f)
{
	(void)f;                                                                                   // 形参按标准库要求保留，实际不使用
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);                              // 阻塞发送 printf 输出的这个字符
	return ch;                                                                                 // 按标准库约定把该字符返回
}

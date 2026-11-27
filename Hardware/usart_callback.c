#include "usart.h"
#include "usart_callback.h"
#include "main.h"



void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART1)     
	{
		USART1_ErrorCallback();
	}
	else if (huart->Instance == USART2)
    {
        USART2_ErrorCallback();
    }
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    
    if (huart->Instance == USART1)
    {
        USART1_RxEventCallback(Size); // 调用 usart1.c 里的函数
    }
    else if (huart->Instance == USART2)
    {
        USART2_RxEventCallback(Size); // 调用 usart2.c 里的函数
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2)
		USART2_RxEventCallback(1);
}



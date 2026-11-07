#ifndef __USART_H__
#define __USART_H__

#include "stm32f10x.h"
#include <stdio.h>

void USART_GPIO_Init(void);
void WaitBusy(USART_TypeDef* USARTx, uint16_t USART_FLAG);
uint8_t USART_ReceiveByteFlag(uint8_t *ReceiveByte);
void USART_SendByte(uint8_t Byte);
uint8_t USART_ReceiveByte(void);
void USART_SendString(char *String);
uint8_t USART_ReceiveString(char *ReceiveString);
int fputc(int ch, FILE *f);

#endif

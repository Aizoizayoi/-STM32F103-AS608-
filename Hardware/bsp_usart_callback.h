#ifndef __USART_CALLBACK_H__
#define __USART_CALLBACK_H__


extern void USART1_ErrorCallback(void);
extern void USART2_ErrorCallback(void);
extern void  USART1_RxEventCallback(uint16_t Size);
extern void  USART2_RxEventCallback(uint16_t Size);
#endif

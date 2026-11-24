#ifndef __AS608_USART_H__
#define __AS608_USART_H__


#define USART2_MAX_RECV_LEN		400					//最大接收缓存字节数
#define USART2_MAX_SEND_LEN		400					//最大发送缓存字节数
#define USART2_RX_EN 			1					//0,不接收;1,接收.

extern uint8_t  USART2_RX_BUF[USART2_MAX_RECV_LEN]; 		//接收缓冲,最大USART2_MAX_RECV_LEN字节
extern uint8_t  USART2_TX_BUF[USART2_MAX_SEND_LEN]; 		//发送缓冲,最大USART2_MAX_SEND_LEN字节
extern uint16_t USART2_RX_STA;   						//接收数据状态

void AS608_USART_GPIO_Init(void);
void USART_Reset(void);

#endif

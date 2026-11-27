#ifndef __BSP_CH340_H__
#define __BSP_CH340_H__

#include <stdint.h>

/* 单帧最多接收 127 字节，缓冲区多留 1 字节存放 '\0' */
#define CH340_Max_RxBuff    127U
#define CH340_RX_Rec_SIZE  (CH340_Max_RxBuff + 1U)

/* 打开接收后，超过这个时间仍没有任何字节，就关闭接收。时间来自 SysTick 的毫秒计数 */
#define CH340_RX_TIMEOUT_MS 30000U

typedef enum
{
	USART_OK        = 0x00U,	/* 收到一帧可打印字符串 */
	USART_ERROR     = 0x01U,	/* 参数无效、启动失败，或去掉换行后为空行 */
	USART_Overflow  = 0x02U,	/* 硬件溢出，或本帧已塞满接收区 */
	USART_illegal   = 0x03U,	/* 含有不可打印字符 */
	USART_NO_DATA   = 0x04U,	/* 这一轮还没有新的一帧 */
	USART_TIMEOUT   = 0x05U		/* 30 秒内没有收到任何字节，接收已关闭 */
} CH340_USART_StatusTypeDef;

CH340_USART_StatusTypeDef CH340_ReceiveString(uint8_t *ReceiveString,uint16_t ReceiveSize);

#endif

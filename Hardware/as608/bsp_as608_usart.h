#ifndef __AS608_USART_H__
#define __AS608_USART_H__

#define AS608_MAX_RxBuff 1024
#define AS608_MAX_TxBuff 1024

typedef enum
{
	AS608_Data_AckOK = 0x00,
	AS608_Data_HeadWrong = 0x01,
	AS608_Data_AddWrong = 0x02,
	AS608_Data_AckCodeWrong = 0x03,
	AS608_Data_LengthWrong = 0x04,
	AS608_Data_Error = 0x05,
}AS608_Data_StateTypeDef;

typedef enum
{
	AS608_Receive_Start = 0x00,
	AS608_Receive_RecDone = 0x01,
}AS608_Receive_StateTypeDef;

extern uint8_t USART2_RX_Receive[AS608_MAX_RxBuff];
extern uint8_t USART2_RX_BUF[AS608_MAX_RxBuff]; 	
extern uint8_t USART2_TX_BUF[AS608_MAX_TxBuff]; 	
extern volatile AS608_Receive_StateTypeDef USART2_RX_STA;


AS608_Data_StateTypeDef AS608_AckVertify(void);
void AS608_BeginReceive(void);
void AS608_Reset(void);

#endif

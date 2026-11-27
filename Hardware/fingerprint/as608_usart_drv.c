#include "fingerprint/as608_usart_drv.h"

#include "usart.h"
#include "usart_callback.h"

#include <string.h>

volatile uint16_t USART2_RX_Size = 0;
AS608_Data_StateTypeDef RxData_State;
volatile AS608_Receive_StateTypeDef USART2_RX_STA;

uint8_t USART2_RX_BUF[AS608_MAX_RxBuff];
uint8_t USART2_RX_Receive[AS608_MAX_RxBuff];

static uint32_t Address=0xFFFFFFFF;

           
static uint16_t rx_len = 0;         


/**
  * @brief  启动数据接收
  * @param  None
  * @retval None
  */

void AS608_BeginReceive(void)
{
	if(huart2.RxState!=HAL_UART_STATE_READY)		//串口在忙，return
		return;
	USART2_RX_STA = AS608_Receive_Start;
	HAL_UARTEx_ReceiveToIdle_DMA(&huart2,USART2_RX_BUF,AS608_MAX_RxBuff);
}

/**
  * @brief  重置转存数组
  * @param  None
  * @retval None
  */
void AS608_Reset(void)
{
	uint32_t primask = __get_PRIMASK();
	__disable_irq();
	
	memset(USART2_RX_Receive, 0, sizeof(USART2_RX_Receive));

	if (!primask)
		__enable_irq();
}
/**
  * @brief  应答合法性验证
  * @param  None
  * @retval AS608_Data_StateTypeDef 返回应答结果
  */
AS608_Data_StateTypeDef AS608_AckVertify(void)
{
	
	if(USART2_RX_Receive[0]!=0xEF || USART2_RX_Receive[1]!=0x01)
	{
		AS608_Reset();
		RxData_State = AS608_Data_HeadWrong;
	}
	if(USART2_RX_Receive[2]!=(uint8_t)(Address>>24) && 
		USART2_RX_Receive[3]!=(uint8_t)(Address>>16) && 
		USART2_RX_Receive[4]!=(uint8_t)(Address>>8) && 
		USART2_RX_Receive[5]!=(uint8_t)(Address))
	{
		AS608_Reset();
		RxData_State = AS608_Data_AddWrong;
	}
	
	if(USART2_RX_Receive[6]!=0x07)
	{
		AS608_Reset();
		RxData_State = AS608_Data_AckCodeWrong;
	}
		
	rx_len=((uint16_t)USART2_RX_Receive[7]<<8)|USART2_RX_Receive[8];
	
	if(rx_len<3 || rx_len>(AS608_MAX_RxBuff-2))
	{
		AS608_Reset();
		RxData_State = AS608_Data_LengthWrong;
	}
	else
		RxData_State= AS608_Data_AckOK;
	return RxData_State ;
}
/**
  * @brief  IDLE回调函数，接收和转存接收到的数据，并返还接收到的数据长度
  * @param  Size 接收到的数据长度
  * @retval None
  */
void USART2_RxEventCallback(uint16_t Size)
{
	if(Size>0 && Size<AS608_MAX_RxBuff)
	{
		memcpy(USART2_RX_Receive,USART2_RX_BUF,Size);
		USART2_RX_Size=Size;
		USART2_RX_STA=AS608_Receive_RecDone;
	}

}
/**
  * @brief  错误回调
  * @param  None
  * @retval None
  */
void USART2_ErrorCallback()
{
	USART2_RX_STA=AS608_Receive_RecDone;
	if (huart2.RxState != HAL_UART_STATE_READY)
		(void)HAL_UART_AbortReceive(&huart2);
	AS608_BeginReceive();
}

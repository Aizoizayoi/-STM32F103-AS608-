#include "matrixkey/pcf8524_drv.h"

#include "i2c.h"

#define PCF8574_DEV_ADDR  (0x20 << 1)


uint8_t g_pcf8574_getflag=0;


pcf8524_StatusTypeDef PCF8574_GetButton(uint8_t *button)
{
	static uint8_t key_held = 0;      // 0: 未按下, 1: 正在按住未松手
//  static uint8_t last_key = 0xFF;   //remember current key
	
	uint8_t line[4] = {0xFE, 0xFD, 0xFB, 0xF7};
	uint8_t i=0;
	
	uint8_t button_ack = 0xFF,init=0xF0;
	uint8_t cur_key= 0xFF;
	
	if(g_pcf8574_getflag==0)
		return pcf8524_BUSY;
	
	g_pcf8574_getflag=0;	
	
	for(i=0;i<4;i++)
	{
		if(HAL_I2C_Master_Transmit(&hi2c2, PCF8574_DEV_ADDR, &line[i], 1, 10)==HAL_OK &&
			HAL_I2C_Master_Receive(&hi2c2, PCF8574_DEV_ADDR, &button_ack, 1, 10)==HAL_OK)
		{
			if((button_ack & 0xF0) !=0xF0)
			{
				cur_key=list(button_ack);
				break;
			}
		
		}
	}
	
	HAL_I2C_Master_Transmit(&hi2c2, PCF8574_DEV_ADDR, &init, 1, 100);
	
	if(cur_key!=0xFF)
	{
		if(!key_held)
		{
			key_held=1;
//			last_key=cur_key;
			*button=cur_key;
			return pcf8524_OK;
		}
		return pcf8524_BUSY;
	}
	else
	{
		key_held=0;
//		last_key=0xFF;
		return pcf8524_ERROR;
	}
	
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == PCF8574_INT_Pin) 
	{
		g_pcf8574_getflag = 1;
  }
}


//P7-0: R4-1 C1-3
uint8_t list(uint8_t rx)
{
	uint8_t button=127;
	switch(rx)
	{
		case 0x7E:
			button=16;
			break;
		case 0xBE:
			button=12;
			break;
		case 0xDE:
			button=8;
			break;
		case 0xEE:
			button=4;
			break;
		case 0x7D:
			button=15;
			break;
		case 0xBD:
			button=11;
			break;
		case 0xDD:
			button=7;
			break;
		case 0xED:
			button=3;
			break;
		case 0x7B:
			button=14;
			break;
		case 0xBB:
			button=10;
			break;
		case 0xDB:
			button=6;
			break;
		case 0xEB:
			button=2;
			break;
		case 0x77:
			button=13;
			break;
		case 0xB7:
			button=9;
			break;
		case 0xD7:
			button=5;
			break;
		case 0xE7:
			button=1;
			break;
		default:
			button=0xFF;
			break;
	}
	return button;
}

#ifndef __SS1603_DRV_H__
#define __SS1603_DRV_H__

#include <stdint.h>

#define OLED_WIDTH      128
#define OLED_HEIGHT     64
#define OLED_PAGE_COUNT (OLED_HEIGHT / 8)
#define ss1603_Add (0x3C)<<1


void OLED_Init(void);
void OLED_WriteCommand(uint8_t CommandByte);
void OLED_WriteData(uint8_t *Data,uint16_t Size);
void OLED_DisplayOn(void);
void OLED_DisplayOff(void);
void OLED_Update(void);
//X: 0~127, Y: 0~63
void OLED_ClearArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height); 
void OLED_Clear(void);
void OLED_FillScreen(void);
void OLED_SetCursor(uint8_t X, uint8_t Page);

//Col: 1~16, Page:1~4
//X: 0~127, Y: 0~63
//Width: 0~127, Height: 0~63
void OLED_ShowImage(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image);
void OLED_ShowPicture(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image);
void OLED_ShowChar(int16_t Page, int16_t Col,uint8_t Char);
void OLED_ShowString(int16_t Page, int16_t Col,char *Char);
void OLED_ShowNum(int16_t Page, int16_t Col,int32_t Number,uint8_t Length);
void OLED_ShowHexNum(int16_t Page, int16_t Col,int32_t Number,uint8_t Length);
void OLED_ShowSignedNum(int16_t Page, int16_t Col,int32_t Number,uint8_t Length);
#endif

#ifndef __OLED_DATA_H__
#define __OLED_DATA_H__

#include <stdint.h>

#define OLED_CHARSET_UTF8
#define OLED_ASCII_MIN   ' '
#define OLED_ASCII_MAX   '~'
#define OLED_ASCII_COUNT 95
//#define OLED_CHARSET_GB2312

typedef struct
{
#ifdef OLED_CHARSET_UTF8
	char Index[5];
#endif
	
#ifdef OLED_CHARSET_GB2312
	char Index[5];
#endif	

	uint8_t Data[32]; //这里规定中文是16*16，即2page*16 line
}ChineseCell_t;

extern const uint8_t OLED_F128X32_Sign[];
extern const ChineseCell_t OLED_CN16x16[] ;
extern const uint8_t OLED_F6x8[][6];
extern const uint8_t OLED_F8x16[][16];
extern const uint8_t Sample16x16[];
extern const uint8_t GoodBoy1[];
extern const uint8_t GoodBoy2[];

uint8_t Return_CN_Size(void);

#endif

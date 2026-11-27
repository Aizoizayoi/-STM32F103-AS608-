
#include "oled/ssd1603_drv.h"

#include "oled/ssd1603_lib.h"
#include "i2c.h"

#include <string.h>

uint8_t ShowStringFlag=0;


static uint8_t OLED_DisplayBuf[OLED_PAGE_COUNT][OLED_WIDTH];

void OLED_WriteCommand(uint8_t CommandByte)
{
	uint8_t Command[2]={0x00};
	Command[1]=CommandByte;
	(void)HAL_I2C_Master_Transmit(&hi2c1,0x78,Command,2,100);
}

void OLED_WriteData(uint8_t *Data,uint16_t Size)
{
	uint8_t tx_buf[Size+1];
	tx_buf[0]=0x40;
	memcpy(&tx_buf[1],Data,Size);
	(void)HAL_I2C_Master_Transmit(&hi2c1,0x78,tx_buf,(uint16_t)(Size+1),100);
}


/**
  * @brief OLED初始化函数
  * @param None
  * @retval None
  */
void OLED_Init(void)
{
//	MX_I2C1_Init();

//	HAL_Delay(100);
	OLED_WriteCommand(0xAE);	//设置显示开启/关闭，0xAE关闭，0xAF开启

	OLED_WriteCommand(0xD5);	//设置显示时钟分频比/振荡器频率
	OLED_WriteCommand(0x80);	//0x00~0xFF

	OLED_WriteCommand(0xA8);	//设置多路复用率
	OLED_WriteCommand(OLED_HEIGHT - 1);	// 32 行屏为 0x1F

	OLED_WriteCommand(0xD3);	//设置显示偏移
	OLED_WriteCommand(0x00);	//0x00~0x7F

	OLED_WriteCommand(0x40);	//设置显示开始行，0x40~0x7F

	OLED_WriteCommand(0xA1);	//设置左右方向，0xA1正常，0xA0左右反置

	OLED_WriteCommand(0xC8);	//设置上下方向，0xC8正常，0xC0上下反置

	OLED_WriteCommand(0xDA);	//设置COM引脚硬件配置
	OLED_WriteCommand(0x12);

	OLED_WriteCommand(0x81);	//设置对比度
	OLED_WriteCommand(0xCF);	//0x00~0xFF

	OLED_WriteCommand(0xD9);	//设置预充电周期
	OLED_WriteCommand(0xF1);

	OLED_WriteCommand(0xDB);	//设置VCOMH取消选择级别
	OLED_WriteCommand(0x30);

	OLED_WriteCommand(0xA4);	//设置整个显示打开/关闭

	OLED_WriteCommand(0xA6);	//设置正常/反色显示，0xA6正常，0xA7反色

	OLED_WriteCommand(0x8D);	//设置充电泵
	OLED_WriteCommand(0x14);

	OLED_WriteCommand(0xAF);	//开启显示

	OLED_Clear();

}

void OLED_DisplayOn(void)
{
	OLED_WriteCommand(0xAF);
}

void OLED_DisplayOff(void)
{
	OLED_WriteCommand(0xAE);
}


/**
  * @brief OLED设置光标位置
  * @param Page 指定光标所在的页，范围：0~(OLED_PAGE_COUNT-1)
  * @param X 光标X轴坐标，范围：0~(OLED_WIDTH-1)
  * @retval None
  */
void OLED_SetCursor(uint8_t X, uint8_t Page)
{
	if (X >= OLED_WIDTH)
	{
		X = OLED_WIDTH - 1;
	}
	if (Page >= OLED_PAGE_COUNT)
	{
		Page = OLED_PAGE_COUNT - 1;
	}

	OLED_WriteCommand(0x00 | (X & 0x0F));
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));
	OLED_WriteCommand(0xB0 | Page);
}

/**
  * @brief Clear the Screen
  * @param None
  * @retval None
  */

void OLED_Clear(void)
{
	uint8_t i=0,j=0;
	for(i=0;i<OLED_PAGE_COUNT;i++)
	{
		for(j=0;j<OLED_WIDTH;j++)
		{
			OLED_DisplayBuf[i][j]=0x00;
		}
	}
	OLED_Update();
}
/**
  * @brief Fill the Screen in white pixel
  * @param None
  * @retval None
  */

void OLED_FillScreen(void)
{
	uint8_t i=0,j=0;
	for(i=0;i<OLED_PAGE_COUNT;i++)
	{
		for(j=0;j<OLED_WIDTH;j++)
		{
			OLED_DisplayBuf[i][j]=0xFF;
		}
	}
	OLED_Update();
}

/**
  * @brief Write the data into GDDRAM and show the data in the screen
  * @param None
  * @retval None
  */

void OLED_Update(void)
{
	uint8_t i;

	for(i=0;i<OLED_PAGE_COUNT;i++)
	{
		OLED_SetCursor(0,i);
		OLED_WriteData(OLED_DisplayBuf[i],128);
	}
}

void OLED_ShowPicture(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image)
{
	OLED_ShowImage(X,Y,Width,Height,Image);
	OLED_Update();
}
/**
  * @brief DisplayOff the OLED Screen
  * @param X: The specific column where data start, can be 0 to 127
  * @param Y: The specific line where data start, can be 0 to 64
  * @param Width: The width pixels image has.
  * @param Height: The Height pixels image has.
  * @param Image: A image arrage in hex
  * @retval None
  */

void OLED_ShowImage(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image)
{
	uint8_t Page,PageCount,Shift,Col;
	uint8_t i,j;
	uint8_t Data;;
	
	PageCount = (Height+7)/8;

	OLED_ClearArea(X,Y,Width,Height);
	for(i=0;i<PageCount;i++)
	{
		Shift= Y%8;
		for(j=0;j<Width;j++)
		{
			Col=X+j;
			Data=Image[i*Width+j];
			Page=(Y/8+i);
			OLED_DisplayBuf[Page][Col]|=(Data<<Shift);
			OLED_DisplayBuf[Page+1][Col]|=(Data>>(8-Shift));
		}
	}
	//OLED_Update();
}
/**
  * @brief Clear specific area on the screen
  * @param X: The specific column where area start, can be 0 to 127
  * @param Y: The specific line where area start, can be 0 to 64
  * @param Width: The width pixels you want to clean.
  * @param Height: The Height pixels you want to clean.
  * @retval None
  */
void OLED_ClearArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
	uint8_t i,j;
	for(i=Y;i<(Y+Height);i++)
	{
		for(j=X;j<(Width+X);j++)
		{
			OLED_DisplayBuf[i/8][j]&=~(0x01<<(i%8));                         
		}
	}
	//OLED_Update();
}
/**
  * @brief Show one char on the screen, which char sized in 8*16
  * @param Page: The page you want to show your char where Page can be 1 to 4
  * @param Col:  The column you want to show your char where col can be 1 to 16
  * @param Char: A Char you want to show.
  * @retval None
  */
void OLED_ShowChar(int16_t Page, int16_t Col,uint8_t Char)
{
	int16_t X,Y,Index;
	
	Y=(Page-1)*16;
	X=(Col-1)*8;
	Index=Char-' ';
	//OLED_ClearArea(X,Y,8,16);
	OLED_ShowImage(X,Y,8,16,OLED_F8x16[Index]);
	if(ShowStringFlag==0)
	{
		OLED_Update();
	}
}
/**
  * @brief Show the String on the screen
  * @param Page: The page you want to show your char where Page can be 1 to 4
  * @param Col:  The column you want to show your char where col can be 1 to 16
  * @param *Char: The String you want to show, where the number of characters in the string should be less than 17.
  * @retval None
  */

void OLED_ShowString(int16_t Page, int16_t Col,char *Char)
{
	int16_t i=0;
  ShowStringFlag=1;
	while(Char[i]!='\0')
	{
		OLED_ShowChar(Page,Col+i,Char[i]);
		i++;
	}
	ShowStringFlag=0;
	OLED_Update();
}

/**
  * @brief To culculate X^Y
	* @param X:under number, where can be 0 to 65534 
	* @param Y:index number, where can be 0 to 65534 
  * @param *Char: The String you want to show, where the number of characters in the string should be less than 17.
  * @retval X^Y
  */

int32_t GetPow(uint16_t X, uint16_t Y)
{
	uint16_t i;
	int32_t Value=1;
	for(i=0;i<Y;i++)
	{
		Value*=X;
	}
	return Value;
}
/**
  * @brief Show number on the screen
  * @param Page: The page you want to show your char where Page can be 1 to 4
  * @param Col:  The column you want to show your char where col can be 1 to 16
  * @param Number: The Number you want to show, where can be 0 to 65534
  * @param Length: The number of digits, where can be 1 to 16
  * @retval None
  */
void OLED_ShowNum(int16_t Page, int16_t Col,int32_t Number,uint8_t Length)
{
	uint8_t i;
	int32_t Divisor,Reminder;
	Reminder=Number;
	for(i=0;i<Length;i++)
	{
		Divisor=Reminder/GetPow(10,Length-1-i);
		OLED_ShowChar(Page,Col+i,Divisor+'0');
		Reminder=(Reminder%GetPow(10,Length-1-i));
	}
	OLED_Update();
}
/**
  * @brief Show Hexnumber on the screen
  * @param Page: The page you want to show your char where Page can be 1 to 4
  * @param Col:  The column you want to show your char where col can be 1 to 16
  * @param Number: The Number you want to show, where can be 0 to 65534. 
	The number should be in hex without '0x' rather than decimalism
  * @param Length: The number of digits, where can be 1 to 16
  * @retval None
  */
void OLED_ShowHexNum(int16_t Page, int16_t Col,int32_t Number,uint8_t Length)
{
	int32_t Num,i;
	Num=Number;
	
	for(i=0;i<Length;i++)
	{
		Num=Number&0x0F;	
		if(Num<=9 & Num>=0)
		{
			OLED_ShowChar(Page,Col+i,Num+'0');
		}
		else if(Num<=15 & Num>=10)
		{
			OLED_ShowChar(Page,Col+i,Num-10+'A');
		}
		Number=Number>>4;
	}
	OLED_Update();
	
}

/**
  * @brief Show number and its sign on the screen
  * @param Page: The page you want to show your char where Page can be 1 to 4
  * @param Col:  The column you want to show your char where col can be 1 to 16
  * @param Number: The Number you want to show, where can be -65534 to 65534
  * @param Length: The number of digits, where can be 1 to 16
  * @retval None
  */
void OLED_ShowSignedNum(int16_t Page, int16_t Col,int32_t Number,uint8_t Length)
{
	if(Number>=0)
	{
		OLED_ShowChar(Page,Col,'+');
		OLED_ShowNum(Page,Col+1,Number,Length);
	}
	else if(Number<0)
	{
		Number=0-Number;
		OLED_ShowChar(Page,Col,'-');
		OLED_ShowNum(Page,Col+1,Number,Length);
	}
	OLED_Update();
}


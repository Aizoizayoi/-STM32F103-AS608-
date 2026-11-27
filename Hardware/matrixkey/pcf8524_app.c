#include "matrixkey/pcf8524_app.h"

#include <stdio.h>
#include <string.h>

#define well_sign '\0'		//结束符
#define illegal_sign '?'

//仅限1-9；超出的部分是特殊字符
//ascii码40-56部分
pcf8524_StatusTypeDef PCF8524_Get1Char(char *ch)
{
	uint8_t get_key=0;
	if(!PCF8574_GetButton(&get_key))
	{
		*ch=get_key+'0';
		return pcf8524_OK;
	}
	*ch='?';
	return pcf8524_ERROR;
}

//S1-S9映射1-9，S10映射0，S16映射‘#’，确认输入
pcf8524_StatusTypeDef PCF8524_Get09Password(uint8_t *pwd,uint8_t pwdlen)
{

	uint8_t get_key=0,map_num=0;
	pcf8524_StatusTypeDef rs_state=pcf8524_BUSY;		//返回状态
	static uint8_t rx_buf[pcf8524_MAX_BUF]={0};
	static uint8_t cur_len=0;
	
	if(!PCF8574_GetButton(&get_key))
	{

		map_num=Key_09Map(get_key);
		
		if(map_num<='9' && map_num>='0')
		{
			rx_buf[cur_len]=map_num;
			printf("curlen=%d,map_num=%d\r\n",cur_len,map_num);
			if(cur_len==pwdlen)
			{
				printf("pws out of length range.try again\r\n");
				rs_state= pcf8524_ERROR;
			}
			rs_state = pcf8524_BUSY;
		}
		else if(map_num==well_sign)
		{
			
			printf("curlen=%d,map_num=%d\r\n",cur_len,map_num);
			if(cur_len==pwdlen)
			{
				strncpy((char *)pwd, (const char *)rx_buf, pwdlen);
				pwd[pwdlen]='\0';
				rs_state = pcf8524_OK;
			}
			else
			{
				printf("wrong password. try again\r\n");
				rs_state= pcf8524_ERROR;
			}
			
		}
		else
		{
			printf("illegal key. try again\r\n");
			rs_state= pcf8524_ERROR;
		}	
		cur_len++;
	}
	
	if(rs_state!= pcf8524_BUSY)
	{
		memset(rx_buf,0,sizeof(rx_buf));				//清空rx_buf缓冲区数据
		cur_len=0;
	}
	
	return rs_state;
}	


uint8_t Key_09Map(uint8_t key)
{
	if(key>=1 && key<=9)
		return key+'0';
	else if(key==10)
		return 0+'0';
	else if(key==16)
		return well_sign;
	else 
		return illegal_sign;
}

#include "stm32f10x.h"                  // Device header
#include "AS608.h"
#include "AS608_USART_Ins.h"
#include "AS608_USART.h"
#include "USART.h"
#include "OLED.h"
#include "Delay.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define AS_CharBuffer1 0x01
#define AS_CharBuffer2 0x02
#define AS_MAX_FingerStore 1023
char AckMessage[32];



void PrintAckMessage(uint8_t Ack)
{
	strcpy(AckMessage, EnsureMessage(Ack));
	printf("%s\r\n", AckMessage);
}



void Get_FingerPrint(uint16_t PageID)
{
	uint8_t Step = 0;
	uint8_t Ack = 0;
	uint8_t GetImageRetry = 0,OtherRetry=0;
	uint8_t FingerID = 1;
	uint16_t Score = 0;
	uint32_t FingerWithout=0;
	
	printf("Please put Your Finger.\r\n");
	while(1)
	{
		
		if(GetImageRetry > 5 || OtherRetry>5)
		{
			printf("Retry too many times.\r\n");
			break;
		}
		switch(Step)
		{
			case 0:
			{
				Ack = AS608_PS_GetImage();	
				if(Ack == 0)
				{
					printf("Image Get Success.\r\n");
					GetImageRetry=0;
					Step = 1;
				}
				else if(Ack == 0x02)
				{
					Delay_ms(50);
					if(FingerWithout++>600)
					{
						printf("Wait finger timeout.\r\n");
						return;
					}
				}
				else
				{	
					PrintAckMessage(Ack);
					GetImageRetry++;
					FingerWithout=0;
					Step=0;
					printf("Please Try again.Left %d/5.\r\n",5-GetImageRetry);
					Delay_ms(100);//留时间抬起手指
				}
				break;
			}
			case 1:
			{
				Ack = AS608_PS_GenChar(FingerID);
				PrintAckMessage(Ack);
				if(Ack == 0 && FingerID == 1)
				{
					printf("Please lift finger, then put Second Time.\r\n");
					FingerWithout=0;
					while(AS608_PS_GetImage()==0x00)		//检测是否抬手
					{
						Delay_ms(50);
						if(++FingerWithout>200)
						{
							printf("Same finger detected. Timeout Return.\r\n");
							return;
						}
					}
					printf("Please put Second Time.\r\n");
					Step = 0;
					FingerID = 2;
					FingerWithout=0;
				}
				else if(Ack == 0 && FingerID == 2)
				{
					Step = 2;
					FingerID = 1;
				}
				else
				{
					GetImageRetry=0;//每次放手指都可以试5次
					OtherRetry++;
					Step = 0;
					FingerID = 1;
				}
				break;
			}
			case 2:
			{
				Ack = AS608_PS_Match(&Score);
				PrintAckMessage(Ack);
				
				if(Ack == 0)
				{
					Step = 3;
				}
				else
				{
					GetImageRetry=0;
					OtherRetry++;
					Step = 0;
					FingerID = 1;
				}
				break;
			}
			case 3:
			{
			
				Ack = AS608_PS_RegModel();
				PrintAckMessage(Ack);
				if(Ack == 0)
				{
					Step = 4;
				}
				else
				{
					GetImageRetry=0;
					OtherRetry++;
					Step = 0;
					FingerID = 1;
				}
				break;
			}
			case 4:
			{
				
				Ack = AS608_PS_StoreChar(AS_CharBuffer1, PageID);
				PrintAckMessage(Ack);
				if(Ack == 0)
				{
					printf("Finger input success.Store in Pos%d\r\n",PageID);
					Step = 0;
					return;
				}
				else
				{
					GetImageRetry=0;
					OtherRetry++;
					Step = 0;
					FingerID = 1;
				}
				break;
			}
			default:
				Step = 0;
				break;
		}
	}
}

void MatchFinger()
{
	uint8_t Ack;
	uint16_t PageID;
	uint16_t MatchScore;
	uint16_t Timeout=0;
	printf("Please put your finger to match.\r\n");
	
	while(1)
	{
		Ack = AS608_PS_GetImage();
		if(Ack==0x00)	//获取到了指纹
			break;
		else if(Ack==0x02)//感应器无手指放入
		{
			Delay_ms(50);
			if(++Timeout>200)
			{
				printf("Wait finger timeout.\r\n");
				return;
			}
				
		}
		else//指纹录入出错
		{
			printf("Get Finger Print Fail. Return.\r\n");
			PrintAckMessage(Ack);
			Timeout=0;
			return;
		}
	}
	

	if(AS608_PS_GenChar(0x01)!=0x00)
	{
		printf("Genchar Fail.Return.\r\n");
		return;
	}
	if(AS608_PS_Search(AS_CharBuffer1,0x00,AS_MAX_FingerStore+1,&PageID,&MatchScore)!=0x00)
	{
		printf("Finger Search Fail.Return.\r\n");
		return;
	}
	if(Ack==0)
	{
		printf("Match. PageID=%u Score=%u\r\n", PageID, MatchScore);
	}
	else
	{
		printf("Not Match.\r\n");
		PrintAckMessage(Ack);
	}
}

void DeleteFinger(uint16_t PageID, uint16_t N)
{
	uint8_t Ackkk;
	Ackkk=AS608_PS_DeletChar(PageID,N);
	if(Ackkk==0)
	{
		printf("Delete No%d to No%d Success.\r\n",PageID,PageID+N-1);
	}
	else
	{
		printf("Delete Fail.\r\n");
	}
}

void AS608_ShowMenu(void)
{
	printf("1.Fingerprint Enter.\r\n");
	printf("2.Fingerprint Match.\r\n");
	printf("3.Fingerprint Delete.\r\n");
	printf("4.Exit.\r\n");
}

/* 把十进制文本转成 0~1024。成功返回 1，并写入 Number */
uint8_t StringToNumber(char *String, uint16_t *Number)
{
	uint16_t i;
	uint16_t num=0;

	if(String==NULL || Number==NULL || String[0]=='\0')//空指针及空字符
		return 0;

	for(i=0;String[i]!='\0';i++)
	{
		if(String[i]<'0' || String[i]>'9')//避免非数字混入
			return 0;
		
		num=(uint16_t)(num*10u+(uint16_t)(String[i]-'0')); //x10后加，再x10，再加，再x10
		if(num>AS_MAX_FingerStore)	//指纹库最大容积0-1023
			return 0;
	}
	*Number=num;
	return 1;
}

/* 用 USART_ReceiveString 收一行文本再转成数字。1=成功，2=内容非法，0=超时 */
uint8_t AS608_ReceiveNumber(uint16_t *Number)
{
	char buf[USART_STR_MAX_LEN];
	uint8_t Result;

	Result=USART_ReceiveString(buf);
	if(Result==2)
	{
		return 0;
	}

	if(Result==0 || StringToNumber(buf,Number)==0)//输入非法或数字超出最大值
	{
		printf("Input Invalid. Please input num from 0-1023.\r\n");
		return 2;
	}
	printf("Receive\r\n");
	return 1;
}


void AS608_Workshop(void)
{
	uint16_t Choice=0,PageID=0;
	uint8_t Retry=0;
	uint8_t ReadResult;

	AS608_ShowMenu();

	while(1)
	{
		ReadResult=AS608_ReceiveNumber(&Choice);
	
		if(ReadResult==0)
		{
			printf("Long time no operation. Exit.\r\n");
			return;
		}
		if(ReadResult!=1 || Choice<1 || Choice>4)
		{
			Retry++;
			if(Retry>=5)
			{
				printf("Too many wrong number. Return\r\n");
				return;
			}
			printf("Invalid choice. Please Retry. Left %d/5.\r\n",5-Retry);
			continue;
		}
		Retry=0;

		switch(Choice)
		{
			case 1:
			{
				printf("Please input the stored position.\r\n");
				if(AS608_ReceiveNumber(&PageID)!=1)
					break;
				printf("--->PageID=%d Input OK.\r\n",PageID);
				Get_FingerPrint(PageID);
				AS608_ShowMenu();
				break;
			}
			case 2:
			{
				MatchFinger();
				AS608_ShowMenu();
				break;
			}
			case 3:
			{
				printf("Please input the ID you want to delete.\r\n");
				if(AS608_ReceiveNumber(&PageID)!=1)
					break;
				printf("--->PageID=%d Input OK.\r\n",PageID);
				DeleteFinger(PageID,1);
				AS608_ShowMenu();
				break;
			}
			case 4:
			{
				printf("Exit Sucess.\r\n");
				return;
			}
			default:
			{
				printf("Invalid choice. Please try again.\r\n");
				break;
			}
		}
	}
}



int main(void)
{


	USART_GPIO_Init();			/* USART1: printf, 9600 */
	AS608_USART_GPIO_Init();	/* USART2: AS608, 57600 */

	printf("USART1 OK\r\n");	/* 先测串口，确认能打印 */
	AS608_Workshop();
	while(1)
	{
		
		
	}
}

#include "main.h"
#include <stdio.h>
#include <string.h> 
#include <stdint.h>
#include "fingerprint/as608_app.h"
#include "fingerprint/as608_drv.h"
#include "ttl2usb/ttl2usb_drv.h"




uint8_t Para[100]={0};
uint8_t Add=0x11,ack=0x12;
uint16_t Head=0x13;




char AckMessage[32];					//响应符对应定义

/**
  * @brief  打印响应结果到串口
  * @param  none
  * @retval None
  */

void PrintAckMessage(fp_AckDefine_t Ack)
{
	strcpy(AckMessage, EnsureMessage(Ack));
	printf("%s\r\n", AckMessage);
}

//返回1024个存储位中，哪些位置存了手指，且存的是哪个手指

/**
  * @brief  录入指纹
  * @param  PageID：要存入的位置，范围：为0-1023
  * @retval None
  */

  
void Get_FingerPrint(uint16_t PageID)
{
	uint32_t start_time = 0;
	as608_GetFinger_Step_t getfinger_step = getfinger_getimage;
	uint32_t samefinger_time = 0;
	uint32_t samefinger_time2 = 0;
	
	fp_AckDefine_t Ack = FP_OK;
	uint8_t GetImageRetry = 0,OtherRetry=0;
	uint8_t CharBufferID = 1;
	uint16_t Score = 0;

	uint32_t SameFinger=0;
	
	printf("Please put Your Finger.\r\n");
	start_time = HAL_GetTick();

	while(HAL_GetTick()-start_time<60000)//60s超时
	{
		
		if(GetImageRetry > 5 || OtherRetry>5)
		{
			printf("Retry too many times.\r\n");
			break;
		}

		switch(getfinger_step)
		{
			case getfinger_getimage:
			{
				Ack = (fp_AckDefine_t)AS608_PS_GetImage();						       //获取指纹图像应答
			
				if(Ack == FP_ERR_NO_FINGER && SameFinger==1)					//无手指，但先前一直放着手指，判断为抬手
				{
					SameFinger=0;
				}			
				
				if(SameFinger==1)					//有手指，但先前一直未抬手，判断为未抬手
				{
					printf("Same finger detected. Please up your finger.\r\n");
					if(HAL_GetTick()-samefinger_time>30000)
					{
						printf("Same finger detected. Timeout Return.\r\n");
						return;
					}
					continue;
				}		
				
				SameFinger=0;	
				if(Ack == FP_OK)										//获取指纹图像成功
				{
					printf("Image Get Success.\r\n");
					GetImageRetry=0;
					getfinger_step = getfinger_genchar;
					samefinger_time=HAL_GetTick();
				}
				else if(Ack == FP_ERR_NO_FINGER)                                //传感器无手指，等待手指放入
				{
					if(HAL_GetTick()-start_time>10000)
					{
						printf("Wait finger timeout.\r\n");
						return;
					}
				}
				else												//获取指纹图像失败
				{	
					if(SameFinger==0)						//初次放手指，指纹识别识别
					{
						samefinger_time=HAL_GetTick();
						SameFinger=1;
						printf("GetImage Result:");
						PrintAckMessage(Ack);
						GetImageRetry++;
					
						printf("Please Try again.Left %d/5.\r\n",5-GetImageRetry);
					}
					continue;
				}
				break;
			}
			case getfinger_genchar:
			{
				Ack = (fp_AckDefine_t)AS608_PS_GenChar(CharBufferID);
				printf("GenChar Result:");
				PrintAckMessage(Ack);

				if((fp_AckDefine_t)AS608_PS_GetImage()==FP_OK && SameFinger==1) 		//未抬手
				{
					if(HAL_GetTick()-samefinger_time2>5000)
					{
						printf("Same finger detected. Timeout Return.\r\n");
						return;
					}
					continue;
				}
				SameFinger=0;
				if(Ack == FP_OK && CharBufferID == 1)
				{
					printf("Please lift finger, then put Second Time.\r\n");
					
					samefinger_time2=HAL_GetTick();
				
					getfinger_step = getfinger_getimage;
					CharBufferID = 2;
					SameFinger=1;
				}
				else if(Ack == FP_OK && CharBufferID == 2)
				{
					getfinger_step = getfinger_match;
					CharBufferID = 1;
				}
				else
				{
					GetImageRetry=0;//每次放手指都可以试5次
					OtherRetry++;
					getfinger_step = getfinger_getimage;
					CharBufferID = 1;
				}
				break;
			}
			case getfinger_match:
			{
				Ack = (fp_AckDefine_t)AS608_PS_Match(&Score);
//				printf("Match Result:");
//				PrintAckMessage(Ack);
				
				if(Ack == FP_OK)
				{
					getfinger_step = getfinger_regmodel;
				}
				else
				{
					GetImageRetry=0;
					OtherRetry++;
					getfinger_step = getfinger_getimage;
					CharBufferID = 1;
				}
				break;
			}
			case getfinger_regmodel:
			{
			
				Ack = (fp_AckDefine_t)AS608_PS_RegModel();
//				printf("RegModel Result:");
//				PrintAckMessage(Ack);
				if(Ack == FP_OK)
				{
					getfinger_step = getfinger_storechar;
				}
				else
				{
					GetImageRetry=0;
					OtherRetry++;
					getfinger_step = getfinger_getimage;
					CharBufferID = 1;
				}
				break;
			}
			case getfinger_storechar:
			{
				
				Ack = (fp_AckDefine_t)AS608_PS_StoreChar(AS_CharBuffer1, PageID);
				printf("StoreChar Result:");
				PrintAckMessage(Ack);
				if(Ack == FP_OK)
				{
					printf("Finger input success.Store in Pos %d\r\n",PageID);
					getfinger_step = getfinger_getimage;
					return;
				}
				else
				{
					GetImageRetry=0;
					OtherRetry++;
					getfinger_step = getfinger_getimage;
					CharBufferID = 1;
				}
				break;
			}
			default:
			{
				getfinger_step = getfinger_getimage;
				break;
			}
				
		}
	}
}
/**
  * @brief  匹配指纹，显示匹配结果
  * @param  none
  * @retval None
  */
void MatchFinger()
{
	fp_AckDefine_t Ack;
	uint16_t PageID;
	uint16_t MatchScore;
	uint16_t Timeout=0;
	printf("Please put your finger to match.\r\n");
	
	while(1)
	{
		Ack = (fp_AckDefine_t)AS608_PS_GetImage();
		if(Ack==FP_OK)	//获取到了指纹
			break;
		else if(Ack==FP_ERR_NO_FINGER)//感应器无手指放入
		{
			HAL_Delay(50);
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
	

	if((fp_AckDefine_t)AS608_PS_GenChar(AS_CharBuffer1)!=FP_OK)
	{
		printf("Genchar Fail.Return.\r\n");
		return;
	}
	if((fp_AckDefine_t)AS608_PS_Search(AS_CharBuffer1,0x00,AS_MAX_FingerStore+1,&PageID,&MatchScore)!=FP_OK)
	{
		printf("Finger Search Fail.Return.\r\n");
		return;
	}
	if(Ack==FP_OK)
	{
		printf("Match. PageID=%u Score=%u\r\n", PageID, MatchScore);
	}
	else
	{
		printf("Not Match.\r\n");
		PrintAckMessage(Ack);
	}
}
/**
  * @brief  删除指纹
  * @param  PageID：要删除的位置，范围：为0-1023
  * @retval None
  */
fp_AckDefine_t DeleteFinger(uint16_t PageID, uint16_t N)
{
	fp_AckDefine_t Ackkk;
	Ackkk=(fp_AckDefine_t)AS608_PS_DeletChar(PageID,N);
	if(Ackkk==FP_OK)
	{
		printf("Delete No%d to No%d Success.\r\n",PageID,PageID+N-1);
	}
	else
	{
		printf("Delete Fail.\r\n");
	}
	return Ackkk;
}

void EmptyFinger()
{
	fp_AckDefine_t Ack;
	Ack=(fp_AckDefine_t)AS608_PS_Empty();
	if(Ack==FP_OK)
	{
		printf("Clear All Fingerprints Success.\r\n");
	}
	else
	{
		printf("Clear All Fingerprints Fail.\r\n");
	}
}

/**
  * @brief  十进制文本转成数字
  * @param  String：串口输入的需要转换的文本
  * @param  Number：转换后的数字
  * @retval Func_OK 并写入 Number；空或非数字返回 Func_error；超出指纹库范围返回 Func_overflow
  */
as608_FunctionState_t StringToNumber(uint8_t *String, uint16_t *Number)
{
	uint16_t i;
	uint16_t num=0;

	if(String==NULL || Number==NULL || String[0]=='\0')//空指针及空字符
		return Func_error;

	for(i=0;String[i]!='\0';i++)
	{
		if(String[i]<'0' || String[i]>'9')//避免非数字混入
			return Func_error;
		
		num=(uint16_t)(num*10u+(uint16_t)(String[i]-'0')); //x10后加，再x10，再加，再x10
		if(num>AS_MAX_FingerStore)	//指纹库最大容积0-1023
			return Func_overflow;
	}
	*Number=num;
	return Func_OK;
}


/**
  * @brief  接收一行文本转成数字，并对数字进行合法判断
  * @param  Number：转换后的数字
  * @retval Func_OK 成功；Func_error 内容非法；Func_overflow 超出范围；Func_timeout 输入超时
  */
as608_FunctionState_t AS608_ReceiveNumber(uint16_t *Number)
{
	uint8_t buf[127];
	CH340_USART_StatusTypeDef Result;
	as608_FunctionState_t num_state;

	Result=CH340_ReceiveString(buf,127);
	if(Result==USART_TIMEOUT)
	{
		printf("USART_TIMEOUT.\r\n");
		return Func_timeout;
	}
	
	if(Result!=USART_OK)
	{
		printf("Input Invalid. Please input num from 0-1023.\r\n");
		return Func_error;
	}

	num_state = StringToNumber(buf, Number);
	if(num_state!=Func_OK)//输入非法或数字超出最大值
	{
		printf("Input Invalid. Please input num from 0-1023.\r\n");
		return num_state;
	}

	return Func_OK;
}

as608_FunctionState_t AS608_ReceiveString(uint8_t *String)
{
	
	CH340_USART_StatusTypeDef Result;

	Result=CH340_ReceiveString(String,126);
	if(Result==USART_TIMEOUT)
	{
		return Func_timeout;
	}
	

	return Func_OK;
}

/**
  * @brief  显示选项菜单
  * @param  none
  * @retval None
  */
 void AS608_ShowMenu(void)
 {
	  printf("------------------ Menu Start ------------------\r\n");
		printf("1.Fingerprint Enter.\r\n");
		printf("2.Fingerprint Match.\r\n");
		printf("3.Fingerprint Delete.\r\n");
		printf("4.Fingerprint Review.\r\n");
		printf("5.Clear all fingerprints.\r\n");
		printf("6.Exit/Return.\r\n");
	  printf("------------------ Menu End ------------------\r\n");
 }

typedef enum
{
	choice_fingerprint_start = 0,
	choice_fingerprint_enter = 1,
	choice_fingerprint_match = 2,
	choice_fingerprint_delete = 3,
	choice_fingerprint_review = 4,
	choice_fingerprint_clear = 5,
	choice_exit = 6,
}FingerPrint_choice_state_t;

static as608_FPMap_t finger_map[AS_FP_MAP_NUM]={0};
void AS608_UI(void)
{
	static FingerPrint_choice_state_t FingerPrint_choice_state = choice_fingerprint_start;
	static uint32_t timeout_time = 0;
	uint16_t delete_PageID=0;
	
	as608_FunctionState_t ReadResult = Func_OK;
	
	uint16_t Choice = 0;
	printf("------------------ Show UI ------------------\r\n");
	
	if(HAL_GetTick()-timeout_time>60000)
	{
		printf("Timeout. Exit.\r\n");
		FingerPrint_choice_state = choice_exit;
		return;
	}

	if(FingerPrint_choice_state == choice_fingerprint_start)
	{
		AS608_ShowMenu();
		timeout_time = HAL_GetTick();
		ReadResult=AS608_ReceiveNumber(&Choice);
		
		if(ReadResult==Func_OK)
		{
			if(Choice==1)
			{
				FingerPrint_choice_state=choice_fingerprint_enter;
			}
			else if(Choice==2)
			{
				FingerPrint_choice_state=choice_fingerprint_match;
			}
			else if(Choice==3)
			{
				FingerPrint_choice_state=choice_fingerprint_delete;
			}
			else if(Choice==4)
			{
				FingerPrint_choice_state=choice_fingerprint_review;
			}
			else if(Choice==5)
			{
				FingerPrint_choice_state=choice_fingerprint_clear;
			}
			else if(Choice==6)
			{
				FingerPrint_choice_state=choice_exit;
			}
			else	
			{
				printf("Invalid choice. Please input again.\r\n");
				return;
			}
		}
		printf("Choice Error. Please input again.\r\n");
		return;
	}
	
	if(FingerPrint_choice_state == choice_fingerprint_enter)
	{
		uint8_t slot;

		for(slot=0; slot<AS_FP_MAP_NUM; slot++)//查找空闲位
		{
			if(finger_map[slot].used==0)
				break;
		}
		if(slot>=AS_FP_MAP_NUM)
		{
			printf("Store filled.\r\n");
			FingerPrint_choice_state=choice_fingerprint_start;
			return;
		}
		
		printf("Please input the stored name.\r\n");
		if(AS608_ReceiveString(finger_map[slot].finger_name)!=Func_OK)
		{
			printf("Input Error. Please input again.\r\n");
			return;
		}
		finger_map[slot].page_id=slot;
		finger_map[slot].used=1;
		printf("--->%s stored in PageID=%d Input OK.\r\n",finger_map[slot].finger_name,slot);
		
		Get_FingerPrint(slot);
		FingerPrint_choice_state=choice_fingerprint_start;
		return;
	}
	
	if(FingerPrint_choice_state == choice_fingerprint_match)
	{
		MatchFinger();
		FingerPrint_choice_state=choice_fingerprint_start;
		return;
	}
	
	if(FingerPrint_choice_state == choice_fingerprint_delete)
	{
		printf("Please input the ID you want to delete.\r\n");
		if(AS608_ReceiveNumber(&delete_PageID)!=Func_OK)
			return;
		if(delete_PageID>=AS_FP_MAP_NUM || finger_map[delete_PageID].used==0)
		{
			printf("PageID=%d not found.\r\n",delete_PageID);
			FingerPrint_choice_state=choice_fingerprint_start;
			return;
		}

		if(DeleteFinger(delete_PageID,1)!=FP_OK)
		{
			FingerPrint_choice_state=choice_fingerprint_start;
			return;
		}
		memset(&finger_map[delete_PageID],0,sizeof(finger_map[delete_PageID]));
		printf("PageID=%d deleted.\r\n",delete_PageID);
		FingerPrint_choice_state=choice_fingerprint_start;
		return;
	}
	
	if(FingerPrint_choice_state == choice_fingerprint_clear)
	{
		EmptyFinger();
		memset(finger_map,0,sizeof(finger_map));
		printf("All fingerprints cleared.\r\n");
		FingerPrint_choice_state=choice_fingerprint_start;
		return;
	}

	if(FingerPrint_choice_state == choice_fingerprint_review)
	{
		uint16_t i;
		uint8_t shown=0;
		for(i=0;i<AS_FP_MAP_NUM;i++)
		{
			if(finger_map[i].used==0)
				continue;
			printf("PageID=%d Name=%s\r\n",finger_map[i].page_id,finger_map[i].finger_name);
			shown++;
		}
		if(shown==0)
			printf("No fingerprint stored.\r\n");
		FingerPrint_choice_state=choice_fingerprint_start;
		return;

	}
	
}
// /**
//   * @brief  功能选择。1录入指纹，2匹配指纹，3删除指纹，4退出程序
//   * @param  none
//   * @retval none
//   */
//  void AS608_UI(void)
//  {
//      uint16_t Choice=0,PageID=0;
//      uint8_t Retry=0;
//      uint8_t ReadResult;
 
//      AS608_ShowMenu();
 
//      while(1)
//      {
//          ReadResult=AS608_ReceiveNumber(&Choice);
     
//          if(ReadResult==Func_timeout)
//          {
//              printf("Long time no operation. Exit.\r\n");
//              return;
//          }
//          if(ReadResult!=Func_OK || Choice<1 || Choice>4)
//          {
//              Retry++;
//              if(Retry>=5)
//              {
//                  printf("Too many wrong number. Return\r\n");
//                  return;
//              }
//              printf("Invalid choice. Please Retry. Left %d/5.\r\n",5-Retry);
//              continue;
//          }
//          Retry=0;
 
//          switch(Choice)
//          {
//              case 1:
//              {
//                  printf("Please input the stored position.\r\n");
//                  if(AS608_ReceiveNumber(&PageID)!=Func_OK)
//                      break;
//                  printf("--->PageID=%d Input OK.\r\n",PageID);
//                  Get_FingerPrint(PageID);
//                  AS608_ShowMenu();
//                  break;
//              }
//              case 2:
//              {
//                  MatchFinger();
//                  AS608_ShowMenu();
//                  break;
//              }
//              case 3:
//              {
//                  printf("Please input the ID you want to delete.\r\n");
//                  if(AS608_ReceiveNumber(&PageID)!=Func_OK)
//                      break;
//                  printf("--->PageID=%d Input OK.\r\n",PageID);
//                  DeleteFinger(PageID,1);
//                  AS608_ShowMenu();
//                  break;
//              }
//              case 4:
//              {
//                  printf("Exit Sucess.\r\n");
//                  return;
//              }
//              default:
//              {
//                  printf("Invalid choice. Please try again.\r\n");
//                  break;
//              }
//          }
//      }
//  }
 
 
